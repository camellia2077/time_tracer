      option = {
        animation: false,
        tooltip: { trigger: 'axis' },
        legend: sharedLegend,
        grid: sharedGrid,
        xAxis: {
          type: 'category',
          name: 'Date',
          nameLocation: 'middle',
          nameGap: 30,
          data: labels
        },
        yAxis: {
          type: 'value',
          name: 'Hours',
          min: 0,
          nameLocation: 'middle',
          nameGap: 44
        },
        series: [
          {
            name: 'Duration (hours)',
            type: 'bar',
            data: hours,
            itemStyle: { color: '#2563eb' },
            barMaxWidth: 26
          },
          {
            name: 'Average/day (hours)',
            type: 'line',
            smooth: false,
            symbol: 'none',
            data: labels.map(() => averageSeconds / 3600.0),
            lineStyle: { width: 2, type: 'dashed', color: '#d97706' }
          }
        ]
      };
