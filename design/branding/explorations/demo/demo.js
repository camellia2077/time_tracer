        const svgWrapper = document.getElementById('svgWrapper');
        const titleEl = document.getElementById('currentAnimTitle');
        const descEl = document.getElementById('animDescription');

        function clearAnimations() {
            svgWrapper.className = 'svg-container';
            void svgWrapper.offsetWidth;
        }

        function setAnimation(btn, animClass, title, description) {
            clearAnimations();
            if (animClass) svgWrapper.classList.add(animClass);
            updateText(title, description);
            updateActiveButton(btn);
        }

        function triggerOnce(btn, animClass, title, description) {
            clearAnimations();
            svgWrapper.classList.add(animClass);
            updateText(title, description);
            updateActiveButton(btn);

            const mainElement = svgWrapper.querySelector(animClass === 'anim-commit-bounce' ? '.svg-arrow' : '.trail-main');
            mainElement.onanimationend = null;
            mainElement.onanimationend = (e) => {
                // 判断动画名称，防止意外重置
                if (e.animationName.includes('charge-main') || e.animationName.includes('bounce-action')) {
                    setTimeout(() => {
                        const resetBtn = document.querySelector('.control-btn'); // 获取第0个按钮
                        setAnimation(resetBtn, '', '静止状态 (Idle)', '操作完成，已重置。');
                    }, animClass === 'anim-commit-bounce' ? 200 : 500);
                }
            };
        }

        function updateText(title, description) {
            titleEl.textContent = title;
            descEl.textContent = description;
        }

        function updateActiveButton(clickedBtn) {
            const buttons = document.querySelectorAll('.control-btn');
            // 复杂的重置逻辑，以兼容我们用不同颜色标注的业务场景按钮
            buttons.forEach(btn => {
                btn.classList.remove('ring-2', 'ring-offset-1', 'ring-slate-400');
                if (!btn.classList.contains('bg-emerald-50') && !btn.classList.contains('bg-blue-50') && !btn.classList.contains('bg-purple-50')) {
                    btn.classList.remove('border-indigo-500', 'text-indigo-600', 'bg-indigo-50');
                }
            });
            if (clickedBtn) {
                // 统一用 ring 强调当前选中状态，避免破坏按钮本身的主题色
                clickedBtn.classList.add('ring-2', 'ring-offset-1', 'ring-slate-400');
            }
        }

        function setColor(clickedBtn, startColor, endColor) {
            document.getElementById('gradStart').style.stopColor = startColor;
            document.getElementById('gradEnd').style.stopColor = endColor;

            const buttons = document.querySelectorAll('.color-btn');
            buttons.forEach(btn => btn.classList.remove('ring-2', 'ring-offset-2', 'ring-slate-400'));
            if (clickedBtn) clickedBtn.classList.add('ring-2', 'ring-offset-2', 'ring-slate-400');
        }

        function setBackground(type) {
            const dataStream = document.getElementById('bgDataStream');
            const timeStream = document.getElementById('bgTimeStream');
            const timeHoriz = document.getElementById('bgTimeHoriz');
            
            // Toggle visibility
            if (dataStream) dataStream.style.opacity = type === 'data' ? '1' : '0';
            if (timeStream) timeStream.style.opacity = type === 'time' ? '1' : '0';
            if (timeHoriz) timeHoriz.style.opacity = type === 'timeHoriz' ? '1' : '0';

            // Update button styles
            const buttons = document.querySelectorAll('.bg-btn');
            buttons.forEach(btn => {
                btn.classList.remove('border-indigo-500', 'text-indigo-700', 'bg-indigo-50');
                btn.classList.add('border-slate-200', 'text-slate-500');
            });

            const activeBtn = document.getElementById(`bg-btn-${type}`);
            if (activeBtn) {
                activeBtn.classList.remove('border-slate-200', 'text-slate-500');
                activeBtn.classList.add('border-indigo-500', 'text-indigo-700', 'bg-indigo-50');
            }
        }