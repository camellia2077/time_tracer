package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

@Composable
internal fun ConfigReportAverageDayBasisCard(
    selected: ReportAverageDayBasis,
    onSelected: (ReportAverageDayBasis) -> Unit
) {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(text = stringResource(R.string.config_title_report_average_day_basis))
            Text(text = stringResource(R.string.config_report_average_day_basis_description))
            ReportAverageDayBasis.entries.forEach { basis ->
                androidx.compose.foundation.layout.Row {
                    RadioButton(selected = selected == basis, onClick = { onSelected(basis) })
                    Text(
                        text = stringResource(
                            if (basis == ReportAverageDayBasis.ACTIVE_DAYS) {
                                R.string.config_report_average_day_basis_active
                            } else {
                                R.string.config_report_average_day_basis_calendar
                            }
                        ),
                        modifier = Modifier.padding(top = 12.dp)
                    )
                }
            }
        }
    }
}
