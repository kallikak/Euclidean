#pragma once

// A basic set of 808 style drum kit samples

float getVolumeFactor(int type);
const char *getInstrumentDesc(int type, bool shortDesc);

#ifdef DK_BASS
extern const unsigned int AudioSampleBassdrum[1249];
#define DK_BASS_ARRAY AudioSampleBassdrum
#endif

#ifdef DK_CLAP
extern const unsigned int AudioSampleClap[1345];
#define DK_CLAP_ARRAY AudioSampleClap
#endif

#ifdef DK_CLAVES
extern const unsigned int AudioSampleClaves[289];
#define DK_CLAVES_ARRAY AudioSampleClaves
#endif

#ifdef DK_CONGA_H
extern const unsigned int AudioSampleHighconga[737];
#define DK_CONGA_H_ARRAY AudioSampleHighconga
#endif

#ifdef DK_CONGA_L
extern const unsigned int AudioSampleLowconga[1505];
#define DK_CONGA_L_ARRAY AudioSampleLowconga
#endif

#ifdef DK_CONGA_M
extern const unsigned int AudioSampleMedconga[865];
#define DK_CONGA_M_ARRAY AudioSampleMedconga
#endif

#ifdef DK_CYMBAL
extern const unsigned int AudioSampleCymbal[3041];
#define DK_CYMBAL_ARRAY AudioSampleCymbal
#endif

#ifdef DK_HAT_C
extern const unsigned int AudioSampleClosedhihat[353];
#define DK_HAT_C_ARRAY AudioSampleClosedhihat
#endif

#ifdef DK_HAT_O
extern const unsigned int AudioSampleOpenhihat[1057];
#define DK_HAT_O_ARRAY AudioSampleOpenhihat
#endif

#ifdef DK_MARACAS
extern const unsigned int AudioSampleMaracas[129];
#define DK_MARACAS_ARRAY AudioSampleMaracas
#endif

#ifdef DK_RIDECYMBAL
extern const unsigned int AudioSampleRidecymbal[449];
#define DK_RIDECYMBAL_ARRAY AudioSampleRidecymbal
#endif

#ifdef DK_RIMSHOT
extern const unsigned int AudioSampleRimshot[161];
#define DK_RIMSHOT_ARRAY AudioSampleRimshot
#endif

#ifdef DK_SNARE
extern const unsigned int AudioSampleSnare[609];
#define DK_SNARE_ARRAY AudioSampleSnare
#endif

#ifdef DK_TOM_H
extern const unsigned int AudioSampleTomtom_high_lv[3361];
#define DK_TOM_H_ARRAY AudioSampleTomtom_high_lv
#endif

#ifdef DK_TOM_L
extern const unsigned int AudioSampleTomtom_low_lv[5153];
#define DK_TOM_L_ARRAY AudioSampleTomtom_low_lv
#endif

#ifdef DK_TOM_M
extern const unsigned int AudioSampleTomtom_med_lv[3841];
#define DK_TOM_M_ARRAY AudioSampleTomtom_med_lv
#endif
