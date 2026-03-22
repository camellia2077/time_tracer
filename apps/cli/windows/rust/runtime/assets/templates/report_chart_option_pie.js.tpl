      const pieData = rows
        .map((row) => ({
          name: String(row?.date ?? ''),
          value: Math.max(0, Number(row?.duration_seconds ?? 0))
        }))
        .filter((item) => item.value > 0);

      if (pieData.length === 0) {
        pieData.push({ name: 'No active data', value: 1 });
      }

      option = {
        animation: false,
        tooltip: {
          trigger: 'item',
          formatter: (params) => {
            const value = Number(params?.value ?? 0);
            const hoursValue = value / 3600.0;
            return `${params?.name ?? ''}<br/>${hoursValue.toFixed(2)} h`;
          }
        },
        legend: { orient: 'vertical', right: 0, top: 'center' },
        series: [
          {
            name: 'Duration',
            type: 'pie',
            radius: ['30%', '68%'],
            center: ['38%', '52%'],
            avoidLabelOverlap: true,
            data: pieData,
            label: {
              formatter: ({ name, value }) => `${name}: ${(Number(value) / 3600.0).toFixed(2)}h`
            }
          }
        ]
      };
