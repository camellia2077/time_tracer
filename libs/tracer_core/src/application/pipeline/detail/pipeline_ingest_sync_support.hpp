#ifndef APPLICATION_PIPELINE_DETAIL_PIPELINE_INGEST_SYNC_SUPPORT_HPP_
#define APPLICATION_PIPELINE_DETAIL_PIPELINE_INGEST_SYNC_SUPPORT_HPP_

namespace tracer_core::application::ports {
class ITimeSheetWriteRepository;
}  // namespace tracer_core::application::ports

namespace tracer::core::application::pipeline {

class PipelineSession;

namespace detail {

auto PersistIngestSyncSnapshot(
    const PipelineSession& context,
    tracer_core::application::ports::ITimeSheetWriteRepository& repository)
    -> void;

auto PersistSingleIngestSyncEntry(
    const PipelineSession& context,
    tracer_core::application::ports::ITimeSheetWriteRepository& repository)
    -> void;

}  // namespace detail
}  // namespace tracer::core::application::pipeline

#endif  // APPLICATION_PIPELINE_DETAIL_PIPELINE_INGEST_SYNC_SUPPORT_HPP_
