#pragma once

const char *getInstrumentDesc(int type, bool shortDesc);

extern unsigned int *drumSample1;
extern unsigned int *drumSample2;
extern unsigned int *drumSample3;
extern unsigned int *drumSample4;

extern int sampleSet;

typedef struct
{
  int beats[4];
} drumKit;

#define DK_OFF        0
#define DK_FIRST      100
extern int DK_LAST;

extern int numKits;
extern drumKit *kits;

void initSamples();

int getNumSampleSets();
char *getSetName(int k);
void loadSampleSet(char *samplesetname);

bool loadSampleDesc(int sampleSet);
void updateSamples(int s1, int s2, int s3, int s4);

extern const unsigned int AudioSampleBassdrum[1249];
#define DK_BASS_ARRAY AudioSampleBassdrum

extern const unsigned int AudioSampleClap[1345];
#define DK_CLAP_ARRAY AudioSampleClap

extern const unsigned int AudioSampleClaves[289];
#define DK_CLAVES_ARRAY AudioSampleClaves

extern const unsigned int AudioSampleHighconga[737];
#define DK_CONGA_H_ARRAY AudioSampleHighconga

extern const unsigned int AudioSampleLowconga[1505];
#define DK_CONGA_L_ARRAY AudioSampleLowconga

extern const unsigned int AudioSampleMedconga[865];
#define DK_CONGA_M_ARRAY AudioSampleMedconga

extern const unsigned int AudioSampleCymbal[3041];
#define DK_CYMBAL_ARRAY AudioSampleCymbal

extern const unsigned int AudioSampleClosedhihat[353];
#define DK_HAT_C_ARRAY AudioSampleClosedhihat

extern const unsigned int AudioSampleOpenhihat[1057];
#define DK_HAT_O_ARRAY AudioSampleOpenhihat

extern const unsigned int AudioSampleMaracas[129];
#define DK_MARACAS_ARRAY AudioSampleMaracas

extern const unsigned int AudioSampleRidecymbal[449];
#define DK_RIDECYMBAL_ARRAY AudioSampleRidecymbal

extern const unsigned int AudioSampleRimshot[161];
#define DK_RIMSHOT_ARRAY AudioSampleRimshot

extern const unsigned int AudioSampleSnare[609];
#define DK_SNARE_ARRAY AudioSampleSnare

extern const unsigned int AudioSampleTomtom_high_lv[3361];
#define DK_TOM_H_ARRAY AudioSampleTomtom_high_lv

extern const unsigned int AudioSampleTomtom_low_lv[5153];
#define DK_TOM_L_ARRAY AudioSampleTomtom_low_lv

extern const unsigned int AudioSampleTomtom_med_lv[3841];
#define DK_TOM_M_ARRAY AudioSampleTomtom_med_lv
