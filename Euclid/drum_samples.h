#pragma once

// A basic set of 808 style drum kit samples

const char *getInstrumentDesc(int type, bool shortDesc);

#ifdef DK_BASS
extern const unsigned int AudioSampleBassdrum[1249];
#define DK_BASS_ARRAY AudioSampleBassdrum
#endif

#ifdef DK_CLAP
extern const unsigned int AudioSampleClap[4609];
#define DK_CLAP_ARRAY AudioSampleClap
#endif

#ifdef DK_CLAVES
extern const unsigned int AudioSampleClaves[609];
#define DK_CLAVES_ARRAY AudioSampleClaves
#endif

#ifdef DK_CONGA_H
extern const unsigned int AudioSampleConga_high[2177];
#define DK_CONGA_H_ARRAY AudioSampleConga_high
#endif

#ifdef DK_CONGA_L
extern const unsigned int AudioSampleConga_low[5153];
#define DK_CONGA_L_ARRAY AudioSampleConga_low
#endif

#ifdef DK_CONGA_M
extern const unsigned int AudioSampleConga_med[3105];
#define DK_CONGA_M_ARRAY AudioSampleConga_med
#endif

#ifdef DK_COWBELL
extern const unsigned int AudioSampleCowbell[3649];
#define DK_COWBELL_ARRAY AudioSampleCowbell
#endif

#ifdef DK_CYMBAL
extern const unsigned int AudioSampleCymbal[33569];
#define DK_CYMBAL_ARRAY AudioSampleCymbal
#endif

#ifdef DK_HAT_C
extern const unsigned int AudioSampleHihat_closed[961];
#define DK_HAT_C_ARRAY AudioSampleHihat_closed
#endif

#ifdef DK_HAT_O
extern const unsigned int AudioSampleHihat_open[2817];
#define DK_HAT_O_ARRAY AudioSampleHihat_open
#endif

#ifdef DK_MARACAS
extern const unsigned int AudioSampleMaracas[385];
#define DK_MARACAS_ARRAY AudioSampleMaracas
#endif

#ifdef DK_RIMSHOT
extern const unsigned int AudioSampleRimshot[385];
#define DK_RIMSHOT_ARRAY AudioSampleRimshot
#endif

#ifdef DK_SNARE
extern const unsigned int AudioSampleSnare[1601];
#define DK_SNARE_ARRAY AudioSampleSnare
#endif

#ifdef DK_TOM_H
extern const unsigned int AudioSampleTomtom_high[3361];
#define DK_TOM_H_ARRAY AudioSampleTomtom_high
#endif

#ifdef DK_TOM_L
extern const unsigned int AudioSampleTomtom_low[6401];
#define DK_TOM_L_ARRAY AudioSampleTomtom_low
#endif

#ifdef DK_TOM_M
extern const unsigned int AudioSampleTomtom_med[3841];
#define DK_TOM_M_ARRAY AudioSampleTomtom_med
#endif
