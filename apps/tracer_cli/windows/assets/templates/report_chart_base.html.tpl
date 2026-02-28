<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>{{PAGE_TITLE}}</title>
  <style>
    :root { color-scheme: light; }
    body { margin: 0; font-family: "Segoe UI", Tahoma, sans-serif; background: linear-gradient(145deg, #f6f8fb, #ffffff); color: #182230; }
    .wrap { max-width: 980px; margin: 0 auto; padding: 20px 16px 28px; }
    .card { background: #fff; border: 1px solid #e5e7eb; border-radius: 12px; padding: 14px; box-shadow: 0 8px 24px rgba(15,23,42,0.06); }
    h1 { margin: 0 0 8px; font-size: 22px; }
    .meta { margin: 0 0 14px; color: #4b5563; font-size: 14px; }
    .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 10px; margin: 12px 0 0; }
    .stat { border: 1px solid #edf2f7; border-radius: 10px; padding: 10px 12px; background: #f8fafc; }
    .label { color: #475569; font-size: 12px; margin-bottom: 4px; }
    .value { font-weight: 600; font-size: 15px; }
    .chart-shell { position: relative; width: 100%; height: clamp(280px, 52vh, 520px); min-height: 280px; margin-top: 8px; }
    #chart-root { display: block; width: 100%; height: 100%; }
    #fallback { margin-top: 10px; color: #b91c1c; font-size: 13px; display: none; }
  </style>
  <script>
{{ECHARTS_SCRIPT}}
  </script>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>{{CHART_TITLE}}</h1>
      <p class="meta">Root: {{ROOT_LABEL}} | Range: {{RANGE_LABEL}}</p>
      <div class="chart-shell"><div id="chart-root"></div></div>
      <div class="stats" id="stats"></div>
      <p id="fallback"></p>
    </div>
  </div>
  <script id="report-chart-data" type="application/json">{{REPORT_CHART_JSON}}</script>
  <script>
    (() => {
      const chartKind = '{{CHART_KIND}}';
      const chartTheme = '{{CHART_THEME}}';
      const raw = document.getElementById('report-chart-data');
      const fallback = document.getElementById('fallback');
      let payload = {};
      try {
        payload = JSON.parse(raw.textContent || '{}');
      } catch (error) {
        fallback.style.display = 'block';
        fallback.textContent = 'Invalid JSON payload: ' + String(error);
        return;
      }

      const rows = Array.isArray(payload.series) ? payload.series.slice() : [];
      rows.sort((lhs, rhs) => {
        const lEpoch = Number(lhs?.epoch_day);
        const rEpoch = Number(rhs?.epoch_day);
        if (Number.isFinite(lEpoch) && Number.isFinite(rEpoch)) {
          return lEpoch - rEpoch;
        }
        const lDate = String(lhs?.date ?? '');
        const rDate = String(rhs?.date ?? '');
        return lDate.localeCompare(rDate);
      });

      const labels = rows.map((row) => String(row?.date ?? ''));
      const seconds = rows.map((row) => Math.max(0, Number(row?.duration_seconds ?? 0)));
      const hours = seconds.map((value) => value / 3600.0);
      const totalSeconds = Number(payload.total_duration_seconds ?? seconds.reduce((sum, value) => sum + value, 0));
      const averageSeconds = Number(payload.average_duration_seconds ?? 0);
      const activeDays = Number(payload.active_days ?? seconds.filter((value) => value > 0).length);
      const rangeDays = Number(payload.range_days ?? rows.length);

      const stats = [
        ['Points', rows.length],
        ['Total (h)', (totalSeconds / 3600.0).toFixed(2)],
        ['Average/day (h)', (averageSeconds / 3600.0).toFixed(2)],
        ['Active days', activeDays],
        ['Range days', rangeDays]
      ];

      const statsRoot = document.getElementById('stats');
      stats.forEach(([label, value]) => {
        const div = document.createElement('div');
        div.className = 'stat';
        div.innerHTML = `<div class="label">${label}</div><div class="value">${value}</div>`;
        statsRoot.appendChild(div);
      });

      if (typeof echarts === 'undefined') {
        fallback.style.display = 'block';
        fallback.textContent = 'ECharts not loaded. Check local embedded script.';
        return;
      }

      const chartRoot = document.getElementById('chart-root');
      const chartShell = chartRoot.parentElement;
      const chart = echarts.init(chartRoot, null, { renderer: 'canvas' });
      const sharedLegend = { top: 0 };
      const sharedGrid = { left: 56, right: 24, top: 42, bottom: 52 };
      const isoDatePattern = /^\d{4}-\d{2}-\d{2}$/;
      const toIsoDate = (value) => {
        const text = String(value ?? '');
        return isoDatePattern.test(text) ? text : '';
      };
      const fromDate = toIsoDate(payload.from_date);
      const toDate = toIsoDate(payload.to_date);
      const datePoints = rows
        .map((row) => toIsoDate(row?.date))
        .filter((value) => value.length > 0);
      const firstDate = datePoints.length > 0 ? datePoints[0] : fromDate;
      const lastDate = datePoints.length > 0 ? datePoints[datePoints.length - 1] : (toDate || firstDate);
      const heatmapData = rows
        .map((row) => {
          const date = toIsoDate(row?.date);
          if (!date) {
            return null;
          }
          const secondsValue = Math.max(0, Number(row?.duration_seconds ?? 0));
          return [date, secondsValue / 3600.0];
        })
        .filter((item) => Array.isArray(item));
      const configuredHeatmapPalette = {{HEATMAP_PALETTE_COLORS_JSON}};
      const configuredHeatmapPositiveHours = {{HEATMAP_POSITIVE_HOURS_JSON}};
      const heatmapPalette = Array.isArray(configuredHeatmapPalette)
        ? configuredHeatmapPalette.map((item) => String(item ?? ''))
        : [];
      const heatmapPositiveHours = Array.isArray(configuredHeatmapPositiveHours)
        ? configuredHeatmapPositiveHours
            .map((item) => Number(item))
            .filter((item) => Number.isFinite(item) && item > 0)
            .sort((lhs, rhs) => lhs - rhs)
        : [];
      const heatmapTooltip = {
        position: 'top',
        formatter: (params) => {
          const point = Array.isArray(params?.data) ? params.data : [];
          const date = String(point[0] ?? '');
          const hoursValue = Number(point[1] ?? 0);
          return `${date}<br/>${hoursValue.toFixed(2)} h`;
        }
      };
      const buildHeatmapPieces = (thresholds, colors) => {
        if (!Array.isArray(colors) || colors.length < 2 || !Array.isArray(thresholds)) {
          return [];
        }
        const sortedThresholds = thresholds
          .filter((item) => Number.isFinite(item) && item > 0)
          .sort((lhs, rhs) => lhs - rhs);
        if (colors.length !== sortedThresholds.length + 1) {
          return [];
        }

        const pieces = [];
        pieces.push({
          lte: 0,
          color: colors[0],
          label: '0h'
        });
        sortedThresholds.forEach((threshold, index) => {
          const previous = index === 0 ? 0 : sortedThresholds[index - 1];
          const isLast = index === sortedThresholds.length - 1;
          pieces.push({
            gt: previous,
            ...(isLast ? {} : { lte: threshold }),
            color: colors[index + 1],
            label: isLast
              ? `> ${previous.toFixed(2)}h`
              : `${previous.toFixed(2)}h - ${threshold.toFixed(2)}h`
          });
        });
        return pieces;
      };
      const heatmapVisualMap = {
        type: 'piecewise',
        pieces: buildHeatmapPieces(heatmapPositiveHours, heatmapPalette),
        orient: 'horizontal',
        left: 'center',
        bottom: 8,
        itemGap: 8
      };
      const commonCalendarStyle = {
        left: 24,
        right: 24,
        bottom: 116,
        splitLine: { show: true, lineStyle: { color: '#dbe3ee', width: 1 } },
        itemStyle: { borderWidth: 1, borderColor: '#dbe3ee' },
        dayLabel: { firstDay: 1, nameMap: 'en' },
        monthLabel: { nameMap: 'en' }
      };
      const yearRange =
        firstDate && lastDate
          ? [firstDate, lastDate]
          : (fromDate && toDate ? [fromDate, toDate] : undefined);

      let option = null;
{{CHART_OPTION_SCRIPT}}
      if (!option) {
        fallback.style.display = 'block';
        fallback.textContent = `Unsupported chart kind: ${chartKind}`;
        return;
      }

      chart.setOption(option);
      window.addEventListener('resize', () => chart.resize(), { passive: true });
    })();
  </script>
</body>
</html>
