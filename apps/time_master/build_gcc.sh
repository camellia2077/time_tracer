# 如果任何命令执行失败，立即退出脚本
set -e

# 切换到脚本所在的目录，这样无论您从哪里运行此脚本，它都能正常工作
cd "$(dirname "$0")"
echo "--- 已切换到脚本目录: $(pwd)"

echo "--- 正在执行 Python 构建脚本 (build.py)..."

# 执行 Python 脚本，并使用 "$@" 将所有命令行参数原封不动地传递过去
# 例如: 运行 ./build.sh clean --package 将会执行 python build.py clean --package
python build.py --gcc "$@"

echo "--- Python 脚本执行完毕。"