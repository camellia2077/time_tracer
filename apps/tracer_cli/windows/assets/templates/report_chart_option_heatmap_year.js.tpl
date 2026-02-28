      if (chartShell) {
        chartShell.style.height = '640px';
      }
      option = {
        animation: false,
        tooltip: heatmapTooltip,
        visualMap: heatmapVisualMap,
        calendar: {
          ...commonCalendarStyle,
          top: 70,
          cellSize: ['auto', 16],
          yearLabel: { show: true },
          range: yearRange
        },
        series: [
          {
            type: 'heatmap',
            coordinateSystem: 'calendar',
            data: heatmapData
          }
        ]
      };
