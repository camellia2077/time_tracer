      if (chartShell) {
        chartShell.style.height = '460px';
      }
      const monthSourceDate = firstDate || fromDate || toDate;
      const monthRange = monthSourceDate ? monthSourceDate.slice(0, 7) : '';
      const monthFilteredData = monthRange
        ? heatmapData.filter((item) => String(item[0]).startsWith(`${monthRange}-`))
        : heatmapData;
      const monthData = monthFilteredData.length > 0 ? monthFilteredData : heatmapData;

      option = {
        animation: false,
        tooltip: heatmapTooltip,
        visualMap: heatmapVisualMap,
        calendar: {
          ...commonCalendarStyle,
          top: 68,
          cellSize: ['auto', 32],
          yearLabel: { show: false },
          range: monthRange || undefined
        },
        series: [
          {
            type: 'heatmap',
            coordinateSystem: 'calendar',
            data: monthData
          }
        ]
      };
