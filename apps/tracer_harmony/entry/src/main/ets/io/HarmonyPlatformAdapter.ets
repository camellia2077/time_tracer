import fs from '@ohos.file.fs';
import util from '@ohos.util';
import hilog from '@ohos.hilog';

const DOMAIN = 0x3200;
const TAG = 'HarmonyPlatformAdapter';

export class HarmonyPlatformAdapter {
  private readonly filesDir: string;

  constructor(context: Context) {
    this.filesDir = context.filesDir;
    hilog.info(DOMAIN, TAG, `Initialized with filesDir: ${this.filesDir}`);
  }

  getDbPath(): string {
    return `${this.filesDir}/time_data.sqlite3`;
  }

  getOutputRoot(): string {
    return `${this.filesDir}/output`;
  }

  getTestInputPath(): string {
    return `${this.filesDir}/test_data`;
  }

  async prepareSandboxEnvironment(): Promise<string> {
    const outputRoot = this.getOutputRoot();
    if (!fs.accessSync(outputRoot)) {
      fs.mkdirSync(outputRoot);
    }
    
    const configPath = `${outputRoot}/interval_processor_config.toml`;
    if (!fs.accessSync(configPath)) {
      const fd = fs.openSync(configPath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
      fs.writeSync(fd.fd, "[processor]\nbatch_max_size = 100\n");
      fs.closeSync(fd);
    }
    
    const inputRoot = this.getTestInputPath();
    if (!fs.accessSync(inputRoot)) {
      fs.mkdirSync(inputRoot);
    }
    
    return configPath;
  }
}
