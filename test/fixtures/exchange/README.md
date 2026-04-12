# Exchange Fixtures

`test/fixtures/exchange/` 预留给 exchange / import / export 能力本身使用的共享小样本。

它的目标不是替代现有 CLI exchange 测试，而是承接“跨测试、跨入口、可复用”的 exchange 资产。

## 和 CLI Exchange 测试的区别

CLI exchange 测试当前更偏向：

1. 在 suite 或测试代码里临时组装输入
2. 验证某个命令面、某次打包/导入/导出流程是否成功
3. 关注 CLI 外部行为、命令参数、输出契约

`test/fixtures/exchange/` 未来更偏向：

1. 放可被多个 exchange 测试复用的小包样本
2. 放 CLI / runtime / file crypto 都能共用的 exchange 输入
3. 固化历史 exchange bug 的最小回归资产
4. 固化 manifest / payload / alias bundle 的代表性错误路径

一句话说：

- CLI exchange 测试更像“怎么跑”
- `test/fixtures/exchange/` 更像“拿什么样本反复跑”

## 什么时候值得正式启用

下面这些情况出现时，就值得把真实样本沉淀到这里：

1. 多个 exchange 测试开始复用同一组小包样本
2. CLI / runtime / file crypto 需要共享同一个 exchange 输入
3. 某个历史 exchange bug 需要固定成仓库内回归资产
4. manifest / payload / alias bundle 的错误路径开始反复出现

## 建议的第一批样本

如果以后要真正启用，这个目录最值得先放的通常不是“大包”，而是小样本：

1. 最小合法 exchange package
2. manifest 缺字段样本
3. 错 alias index 样本
4. payload 缺失样本
5. import / export 兼容样本

## 边界

1. 不把 CLI suite 自己的一次性临时输出直接搬进这里
2. 不把大体积、低复用的打包产物默认放进这里
3. 只有当样本具有“跨测试复用价值”时，才沉淀到这里
