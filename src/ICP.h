#include <pcl/registration/icp.h>
#include <pcl/correspondence.h>
#include <pcl/PCLPointCloud2.h>
#include <iostream>

template <typename PointSource, typename PointTarget, typename Scalar = float>
class ICP : public pcl::IterativeClosestPointWithNormals<PointSource, PointTarget, Scalar> {
public:
    using PointCloudSource = typename IterativeClosestPoint<PointSource, PointTarget, Scalar>::PointCloudSource;
    using PointCloudSourcePtr = typename PointCloudSource::Ptr;
    using PointCloudSourceConstPtr = typename PointCloudSource::ConstPtr;

    using PointCloudTarget = typename IterativeClosestPoint<PointSource, PointTarget, Scalar>::PointCloudTarget;
    using PointCloudTargetPtr = typename PointCloudTarget::Ptr;
    using PointCloudTargetConstPtr = typename PointCloudTarget::ConstPtr;
    using Matrix4 = typename IterativeClosestPoint<PointSource, PointTarget, Scalar>::Matrix4;

    using IterativeClosestPoint<PointSource, PointTarget, Scalar>::reg_name_;
    using IterativeClosestPoint<PointSource, PointTarget, Scalar>::
        transformation_estimation_;
    using IterativeClosestPoint<PointSource, PointTarget, Scalar>::
        correspondence_rejectors_;

    using Ptr = std::shared_ptr<IterativeClosestPoint<PointSource, PointTarget, Scalar>>;
    using ConstPtr = std::shared_ptr<const IterativeClosestPoint<PointSource, PointTarget, Scalar>>;
    using Matrix4 = typename IterativeClosestPointWithNormals<PointSource, PointTarget, Scalar>::Matrix4;

    using UpdateVisualizerCallbackSignature = void(size_t,
                                                const Matrix4&,
                                                const Matrix4&,
                                                const pcl::CorrespondencesPtr&,
                                                const float score);

    std::function<UpdateVisualizerCallbackSignature> update_visualizer_;

  ICP()
  {
    reg_name_ = "ICP";
    setUseSymmetricObjective(false);
    setEnforceSameDirectionNormals(true);
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
      return correspondences_;
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

      nr_iterations_ = 0;
      converged_ = false;

      // Initialise final transformation to the guessed one
      final_transformation_ = guess;

      // If the guessed transformation is non identity
      if (guess != Matrix4::Identity())
      {
          input_transformed->resize(input_->size());
          // Apply guessed transformation prior to search for neighbours
          transformCloud(*input_, *input_transformed, guess);
      }
      else
          *input_transformed = *input_;

      transformation_ = Matrix4::Identity();

      // Make blobs if necessary
      determineRequiredBlobData();
      PCLPointCloud2::Ptr target_blob(new PCLPointCloud2);
      if (need_target_blob_)
          pcl::toPCLPointCloud2(*target_, *target_blob);

      // Pass in the default target for the Correspondence Estimation/Rejection code
      correspondence_estimation_->setInputTarget(target_);
      if (correspondence_estimation_->requiresTargetNormals())
          correspondence_estimation_->setTargetNormals(target_blob);
      // Correspondence Rejectors need a binary blob
      for (std::size_t i = 0; i < correspondence_rejectors_.size(); ++i)
      {
          registration::CorrespondenceRejector::Ptr &rej = correspondence_rejectors_[i];
          if (rej->requiresTargetPoints())
              rej->setTargetPoints(target_blob);
          if (rej->requiresTargetNormals() && target_has_normals_)
              rej->setTargetNormals(target_blob);
      }

      convergence_criteria_->setMaximumIterations(max_iterations_);
      convergence_criteria_->setRelativeMSE(euclidean_fitness_epsilon_);
      convergence_criteria_->setTranslationThreshold(transformation_epsilon_);
      if (transformation_rotation_epsilon_ > 0)
          convergence_criteria_->setRotationThreshold(transformation_rotation_epsilon_);
      else
          convergence_criteria_->setRotationThreshold(1.0 - transformation_epsilon_);

      // Repeat until convergence
      do
      {
          // Get blob data if needed
          PCLPointCloud2::Ptr input_transformed_blob;
          if (need_source_blob_)
          {
              input_transformed_blob.reset(new PCLPointCloud2);
              toPCLPointCloud2(*input_transformed, *input_transformed_blob);
          }
          // Save the previously estimated transformation
          previous_transformation_ = transformation_;

          // Set the source each iteration, to ensure the dirty flag is updated
          correspondence_estimation_->setInputSource(input_transformed);
          if (correspondence_estimation_->requiresSourceNormals())
              correspondence_estimation_->setSourceNormals(input_transformed_blob);
          // Estimate correspondences
          if (use_reciprocal_correspondence_)
              correspondence_estimation_->determineReciprocalCorrespondences(
                  *correspondences_, corr_dist_threshold_);
          else
              correspondence_estimation_->determineCorrespondences(*correspondences_,
                                                                   corr_dist_threshold_);

          // if (correspondence_rejectors_.empty ())
          CorrespondencesPtr temp_correspondences(new Correspondences(*correspondences_));
          for (std::size_t i = 0; i < correspondence_rejectors_.size(); ++i)
          {
              registration::CorrespondenceRejector::Ptr &rej = correspondence_rejectors_[i];
              PCL_DEBUG("Applying a correspondence rejector method: %s.\n",
                        rej->getClassName().c_str());
              if (rej->requiresSourcePoints())
                  rej->setSourcePoints(input_transformed_blob);
              if (rej->requiresSourceNormals() && source_has_normals_)
                  rej->setSourceNormals(input_transformed_blob);
              rej->setInputCorrespondences(temp_correspondences);
              rej->getCorrespondences(*correspondences_);
              // Modify input for the next iteration
              if (i < correspondence_rejectors_.size() - 1)
                  *temp_correspondences = *correspondences_;
          }

          std::size_t cnt = correspondences_->size();
          // Check whether we have enough correspondences
          if (static_cast<int>(cnt) < min_number_correspondences_)
          {
              PCL_ERROR("[pcl::%s::computeTransformation] Not enough correspondences found. "
                        "Relax your threshold parameters.\n",
                        getClassName().c_str());
              convergence_criteria_->setConvergenceState(
                  pcl::registration::DefaultConvergenceCriteria<
                      Scalar>::CONVERGENCE_CRITERIA_NO_CORRESPONDENCES);
              converged_ = false;
              break;
          }

          // Estimate the transform
          transformation_estimation_->estimateRigidTransformation(
              *input_transformed, *target_, *correspondences_, transformation_);

          // Transform the data
          transformCloud(*input_transformed, *input_transformed, transformation_);

          // Obtain the final transformation
          final_transformation_ = transformation_ * final_transformation_;

          ++nr_iterations_;

        // //   Update the vizualization of icp convergence
          if (update_visualizer_ != 0)
            update_visualizer_(nr_iterations_, transformation_, final_transformation_, correspondences_, getFitnessScore());

          converged_ = static_cast<bool>((*convergence_criteria_));
      } while (convergence_criteria_->getConvergenceState() ==
               pcl::registration::DefaultConvergenceCriteria<
                   Scalar>::CONVERGENCE_CRITERIA_NOT_CONVERGED);

      // Transform the input cloud using the final transformation
      PCL_DEBUG("Transformation "
                "is:\n\t%5f\t%5f\t%5f\t%5f\n\t%5f\t%5f\t%5f\t%5f\n\t%5f\t%5f\t%5f\t%5f\n\t%"
                "5f\t%5f\t%5f\t%5f\n",
                final_transformation_(0, 0),
                final_transformation_(0, 1),
                final_transformation_(0, 2),
                final_transformation_(0, 3),
                final_transformation_(1, 0),
                final_transformation_(1, 1),
                final_transformation_(1, 2),
                final_transformation_(1, 3),
                final_transformation_(2, 0),
                final_transformation_(2, 1),
                final_transformation_(2, 2),
                final_transformation_(2, 3),
                final_transformation_(3, 0),
                final_transformation_(3, 1),
                final_transformation_(3, 2),
                final_transformation_(3, 3));

      // Copy all the values
      output = *input_;
      // Transform the XYZ + normals
      transformCloud(*input_, output, final_transformation_);
  }
};