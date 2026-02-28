import tracer_napi from 'libentry.so';
import { HarmonyPlatformAdapter } from '../io/HarmonyPlatformAdapter';
import hilog from '@ohos.hilog';

const DOMAIN = 0x3200;
const TAG = 'TimeTracerCore';

interface IngestRequest {
  input_path: string;
  date_check_mode: string;
  save_processed_output: boolean;
}

interface TreeRequest {
  period: string;
  period_argument: string;
  list_roots: boolean;
}

export class TimeTracerCore {
  private handle: object | null = null;
  private adapter: HarmonyPlatformAdapter;

  constructor(adapter: HarmonyPlatformAdapter) {
    this.adapter = adapter;
  }

  async initialize(): Promise<boolean> {
    if (this.handle) return true;
    
    try {
      const configPath = await this.adapter.prepareSandboxEnvironment();
      const dbPath = this.adapter.getDbPath();
      const outputRoot = this.adapter.getOutputRoot();
      
      this.handle = tracer_napi.createRuntime(dbPath, outputRoot, configPath);
      hilog.info(DOMAIN, TAG, 'Runtime created successfully.');
      return true;
    } catch (e) {
      hilog.error(DOMAIN, TAG, `Failed to initialize runtime: ${(e as Error).message}`);
      return false;
    }
  }

  async ingestData(inputPath: string): Promise<string> {
    if (!this.handle) throw new Error("Runtime not initialized");
    
    const request: IngestRequest = {
      input_path: inputPath,
      date_check_mode: "none",
      save_processed_output: false
    };
    
    return new Promise((resolve, reject) => {
      try {
        const responseJson = tracer_napi.ingestJson(this.handle!, JSON.stringify(request));
        resolve(responseJson);
      } catch (e) {
        reject(e as Error);
      }
    });
  }

  async queryMonthTree(period: string): Promise<string> {
    if (!this.handle) throw new Error("Runtime not initialized");
    
    const request: TreeRequest = {
      period: "month",
      period_argument: period,
      list_roots: false
    };
    
    return new Promise((resolve, reject) => {
      try {
        const responseJson = tracer_napi.treeJson(this.handle!, JSON.stringify(request));
        resolve(responseJson);
      } catch (e) {
        reject(e as Error);
      }
    });
  }
}
