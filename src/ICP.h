#include <pcl/registration/icp.h>
#include <pcl/correspondence.h>
#include <pcl/PCLPointCloud2.h>

template <typename PointSource, typename PointTarget, typename Scalar = float>
class ICP : public pcl::IterativeClosestPointWithNormals<PointSource, PointTarget, Scalar> {
public:
    using PointCloudSource = typename pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>::PointCloudSource;
    using PointCloudSourcePtr = typename PointCloudSource::Ptr;
    using PointCloudSourceConstPtr = typename PointCloudSource::ConstPtr;

    using PointCloudTarget = typename pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>::PointCloudTarget;
    using PointCloudTargetPtr = typename PointCloudTarget::Ptr;
    using PointCloudTargetConstPtr = typename PointCloudTarget::ConstPtr;
    using Matrix4 = typename pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>::Matrix4;

    using pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>::reg_name_;
    using pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>::transformation_estimation_;
    using pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>::correspondence_rejectors_;

    using Ptr = std::shared_ptr<pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>>;
    using ConstPtr = std::shared_ptr<const pcl::IterativeClosestPoint<PointSource, PointTarget, Scalar>>;
    using Matrix4 = typename pcl::IterativeClosestPointWithNormals<PointSource, PointTarget, Scalar>::Matrix4;

    using UpdateVisualizerCallbackSignature = void(size_t,
                                                const Matrix4&,
                                                const Matrix4&,
                                                const pcl::CorrespondencesPtr&,
                                                const float score);

    std::function<UpdateVisualizerCallbackSignature> update_visualizer_;

  ICP()
  {
    reg_name_ = "ICP";
    this->setUseSymmetricObjective(false);
    this->setEnforceSameDirectionNormals(true);
  };

  virtual ~ICP() {}

virtual void
  transformCloud(const PointCloudSource& input,
                 PointCloudSource& output,
                 const Matrix4& transform) override
    {
        pcl::transformPointCloudWithNormals(input, output, transform);
    }

  inline pcl::CorrespondencesPtr getCorrespondences()
  {
      return this->correspondences_;
  }

  inline bool
  registerVisualizationCallback(
      std::function<UpdateVisualizerCallbackSignature>& visualizerCallback)
  {
    if (visualizerCallback) {
      update_visualizer_ = visualizerCallback;
      return (true);
    }
    return (false);
  }

  void computeTransformation(PointCloudSource &output, const Matrix4 &guess) override
  {
      using namespace pcl;
      // Point cloud containing the correspondences of each point in <input, indices>
      PointCloudSourcePtr input_transformed(new PointCloudSource);

      this->nr_iterations_ = 0;
      this->converged_ = false;

      // Initialise final transformation to the guessed one
      this->final_transformation_ = guess;

      // If the guessed transformation is non identity
      if (guess != Matrix4::Identity())
      {
          input_transformed->resize(this->input_->size());
          // Apply guessed transformation prior to search for neighbours
          transformCloud(*this->input_, *input_transformed, guess);
      }
      else
          *input_transformed = *this->input_;

      this->transformation_ = Matrix4::Identity();

      // Make blobs if necessary
      this->determineRequiredBlobData();
      PCLPointCloud2::Ptr target_blob(new PCLPointCloud2);
      if (this->need_target_blob_)
          pcl::toPCLPointCloud2(*this->target_, *target_blob);

      // Pass in the default target for the Correspondence Estimation/Rejection code
      this->correspondence_estimation_->setInputTarget(this->target_);
      if (this->correspondence_estimation_->requiresTargetNormals())
          this->correspondence_estimation_->setTargetNormals(target_blob);
      // Correspondence Rejectors need a binary blob
      for (std::size_t i = 0; i < this->correspondence_rejectors_.size(); ++i)
      {
          registration::CorrespondenceRejector::Ptr &rej = correspondence_rejectors_[i];
          if (rej->requiresTargetPoints())
              rej->setTargetPoints(target_blob);
          if (rej->requiresTargetNormals() && this->target_has_normals_)
              rej->setTargetNormals(target_blob);
      }

      this->convergence_criteria_->setMaximumIterations(this->max_iterations_);
      this->convergence_criteria_->setRelativeMSE(this->euclidean_fitness_epsilon_);
      this->convergence_criteria_->setTranslationThreshold(this->transformation_epsilon_);
      if (this->transformation_rotation_epsilon_ > 0)
          this->convergence_criteria_->setRotationThreshold(this->transformation_rotation_epsilon_);
      else
          this->convergence_criteria_->setRotationThreshold(1.0 - this->transformation_epsilon_);

      // Repeat until convergence
      do
      {
          // Get blob data if needed
          PCLPointCloud2::Ptr input_transformed_blob;
          if (this->need_source_blob_)
          {
              input_transformed_blob.reset(new PCLPointCloud2);
              toPCLPointCloud2(*input_transformed, *input_transformed_blob);
          }
          // Save the previously estimated transformation
          this->previous_transformation_ = this->transformation_;

          // Set the source each iteration, to ensure the dirty flag is updated
          this->correspondence_estimation_->setInputSource(input_transformed);
          if (this->correspondence_estimation_->requiresSourceNormals())
              this->correspondence_estimation_->setSourceNormals(input_transformed_blob);
          // Estimate correspondences
          if (this->use_reciprocal_correspondence_)
              this->correspondence_estimation_->determineReciprocalCorrespondences(
                  *this->correspondences_, this->corr_dist_threshold_);
          else
              this->correspondence_estimation_->determineCorrespondences(*this->correspondences_,
                                                                   this->corr_dist_threshold_);

          // if (correspondence_rejectors_.empty ())
          CorrespondencesPtr temp_correspondences(new Correspondences(*this->correspondences_));
          for (std::size_t i = 0; i < this->correspondence_rejectors_.size(); ++i)
          {
              registration::CorrespondenceRejector::Ptr &rej = this->correspondence_rejectors_[i];
              PCL_DEBUG("Applying a correspondence rejector method: %s.\n",
                        rej->getClassName().c_str());
              if (rej->requiresSourcePoints())
                  rej->setSourcePoints(input_transformed_blob);
              if (rej->requiresSourceNormals() && this->source_has_normals_)
                  rej->setSourceNormals(input_transformed_blob);
              rej->setInputCorrespondences(temp_correspondences);
              rej->getCorrespondences(*this->correspondences_);
              // Modify input for the next iteration
              if (i < this->correspondence_rejectors_.size() - 1)
                  *temp_correspondences = *this->correspondences_;
          }

          std::size_t cnt = this->correspondences_->size();
          // Check whether we have enough correspondences
          if (static_cast<int>(cnt) < this->min_number_correspondences_)
          {
              PCL_ERROR("[pcl::%s::computeTransformation] Not enough correspondences found. "
                        "Relax your threshold parameters.\n",
                        this->getClassName().c_str());
              this->convergence_criteria_->setConvergenceState(
                  pcl::registration::DefaultConvergenceCriteria<
                      Scalar>::CONVERGENCE_CRITERIA_NO_CORRESPONDENCES);
              this->converged_ = false;
              break;
          }

          // Estimate the transform
          this->transformation_estimation_->estimateRigidTransformation(
              *input_transformed, *this->target_, *this->correspondences_, this->transformation_);

          // Transform the data
          transformCloud(*input_transformed, *input_transformed, this->transformation_);

          // Obtain the final transformation
          this->final_transformation_ = this->transformation_ * this->final_transformation_;

          ++this->nr_iterations_;

        // //   Update the vizualization of icp convergence
          if (this->update_visualizer_ != 0)
            this->update_visualizer_(this->nr_iterations_, this->transformation_, this->final_transformation_, this->correspondences_, this->getFitnessScore());

          this->converged_ = static_cast<bool>((*this->convergence_criteria_));
      } while (this->convergence_criteria_->getConvergenceState() ==
               pcl::registration::DefaultConvergenceCriteria<
                   Scalar>::CONVERGENCE_CRITERIA_NOT_CONVERGED);

      // Transform the input cloud using the final transformation
      PCL_DEBUG("Transformation "
                "is:\n\t%5f\t%5f\t%5f\t%5f\n\t%5f\t%5f\t%5f\t%5f\n\t%5f\t%5f\t%5f\t%5f\n\t%"
                "5f\t%5f\t%5f\t%5f\n",
                this->final_transformation_(0, 0),
                this->final_transformation_(0, 1),
                this->final_transformation_(0, 2),
                this->final_transformation_(0, 3),
                this->final_transformation_(1, 0),
                this->final_transformation_(1, 1),
                this->final_transformation_(1, 2),
                this->final_transformation_(1, 3),
                this->final_transformation_(2, 0),
                this->final_transformation_(2, 1),
                this->final_transformation_(2, 2),
                this->final_transformation_(2, 3),
                this->final_transformation_(3, 0),
                this->final_transformation_(3, 1),
                this->final_transformation_(3, 2),
                this->final_transformation_(3, 3));

      // Copy all the values
      output = *this->input_;
      // Transform the XYZ + normals
      transformCloud(*this->input_, output, this->final_transformation_);
  }
};