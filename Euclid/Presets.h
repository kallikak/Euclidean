#pragma once

#define ROM_PRESETS 10
#define MAX_PRESETS (ROM_PRESETS + 100)
#define SET_MANUAL (MAX_PRESETS+1)
#define SET_DEFAULT (MAX_PRESETS+2)
#define CHOOSE_KIT (MAX_PRESETS+3)
#define DEFAULT_CONFIG -1

void setupPresets();

int nextPreset(int i);
int prevPreset(int i);
int nextFreePreset();

const char *getROMPresetName(int i);

bool deletePresetSD(int i);
bool savePresetSD(int i, config_ptr cfg);
bool loadPresetSD(int i, config_ptr cfg);
void dumpPresetSD(int i);
void dumpAllPresetsSD();
void restorePresetSD(int i);
void restoreFromBackupSD();
bool getTempPresetFile(int i, File *file);
void convertTempPresetFile(int i, File *file);
