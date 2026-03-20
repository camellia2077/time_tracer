package com.example.tracer

internal fun buildTracerTabLifecycleArgs(
    controller: RuntimeGateway,
    recordViewModel: RecordViewModel,
    configViewModel: ConfigViewModel,
    onValidMappingNamesChanged: (Set<String>) -> Unit
): TracerTabLifecycleArgs = TracerTabLifecycleArgs(
    controller = controller,
    recordViewModel = recordViewModel,
    configViewModel = configViewModel,
    recordStatusText = { recordViewModel.uiState.statusText },
    onValidMappingNamesChanged = onValidMappingNamesChanged
)

internal fun dispatchTracerCoordinatorEvent(
    event: TracerCoordinatorEvent,
    selectedTab: TracerTab,
    tabLifecycleArgs: () -> TracerTabLifecycleArgs,
    onTabChanged: (TracerTab) -> Unit
) {
    when (event) {
        is TracerCoordinatorEvent.SelectTab -> {
            val nextTab = event.tab
            if (nextTab == selectedTab) {
                return
            }
            TracerTabRegistry.onLeave(selectedTab, tabLifecycleArgs())
            onTabChanged(nextTab)
        }
    }
}
