import { HarmonyPlatformAdapter } from '../io/HarmonyPlatformAdapter';
import { TimeTracerCore } from '../core/TimeTracerCore';

interface BaseResponse {
  ok: boolean;
  error_message?: string;
}

interface TreeNode {
  name: string;
  duration_seconds?: number;
  children?: TreeNode[];
}

interface TreeResponse extends BaseResponse {
  nodes?: TreeNode[];
}

@Entry
@Component
struct Index {
  @State monthQuery: string = '2026-01';
  @State resultText: string = 'No results yet.';
  @State statusText: string = 'Ready';
  
  private adapter: HarmonyPlatformAdapter | null = null;
  private core: TimeTracerCore | null = null;

  aboutToAppear() {
    this.adapter = new HarmonyPlatformAdapter(getContext(this));
    this.core = new TimeTracerCore(this.adapter);
    
    this.core.initialize().then(success => {
      if (success) {
        this.statusText = 'Core Initialized';
      } else {
        this.statusText = 'Core Init Failed';
      }
    });
  }

  async handleIngest() {
    if (!this.core || !this.adapter) return;
    
    try {
      this.statusText = 'Ingesting data...';
      const inputPath = this.adapter.getTestInputPath();
      const resultJson = await this.core.ingestData(inputPath);
      
      const response = JSON.parse(resultJson) as BaseResponse;
      if (response.ok) {
        this.statusText = 'Ingest OK';
      } else {
        this.statusText = `Ingest Failed: ${response.error_message}`;
      }
    } catch (e) {
      this.statusText = `Error: ${(e as Error).message}`;
    }
  }

  async handleQuery() {
    if (!this.core) return;
    
    try {
      this.statusText = 'Querying...';
      this.resultText = 'Loading...';
      
      const resultJson = await this.core.queryMonthTree(this.monthQuery);
      const response = JSON.parse(resultJson) as TreeResponse;
      
      if (response.ok) {
        // Format nodes basic
        let output = "";
        if (response.nodes && response.nodes.length > 0) {
          response.nodes.forEach((node: TreeNode) => {
            output += `${node.name}: ${node.duration_seconds || 0}s\n`;
            if (node.children) {
              node.children.forEach((child: TreeNode) => {
                 output += `  - ${child.name}: ${child.duration_seconds || 0}s\n`;
              });
            }
          });
          this.resultText = output || "Empty nodes";
        } else {
           this.resultText = "No data found for period.";
        }
        this.statusText = 'Query OK';
      } else {
        this.statusText = `Query Failed: ${response.error_message}`;
        this.resultText = '';
      }
    } catch (e) {
       this.statusText = `Error: ${(e as Error).message}`;
    }
  }

  build() {
    Column() {
      // Header
      Text('Time Tracer Harmony')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20, bottom: 20 })

      // Status
      Text(`Status: ${this.statusText}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 20 })

      // Action 1: Ingest
      Button('1. Ingest Test Data (test/data/2026)')
        .width('80%')
        .height(40)
        .margin({ bottom: 20 })
        .onClick(() => {
          this.handleIngest();
        })

      // Action 2: Query
      Row() {
        TextInput({ text: this.monthQuery, placeholder: 'YYYY-MM' })
          .width('50%')
          .onChange((value: string) => {
            this.monthQuery = value;
          })
        
        Button('2. Query Month')
          .width('30%')
          .margin({ left: 10 })
          .onClick(() => {
            this.handleQuery();
          })
      }
      .width('80%')
      .justifyContent(FlexAlign.SpaceBetween)
      .margin({ bottom: 20 })

      // Result Area
      Scroll() {
        Text(this.resultText)
          .fontSize(16)
          .width('90%')
          .padding(10)
          .backgroundColor('#F5F5F5')
          .borderRadius(8)
      }
      .width('100%')
      .layoutWeight(1)
      .margin({ bottom: 20 })
    }
    .width('100%')
    .height('100%')
    .alignItems(HorizontalAlign.Center)
  }
}
