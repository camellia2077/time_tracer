export const createRuntime: (dbPath: string, outputRoot: string, configPath: string) => object;
export const ingestJson: (handle: object, requestJson: string) => string;
export const treeJson: (handle: object, requestJson: string) => string;