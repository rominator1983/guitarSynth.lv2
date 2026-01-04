#include "lv2/core/lv2.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define URI "http://lv2plug.in/plugins/guitarSynth"

typedef enum
{
   GAIN = 0,
   THRESHOLD = 1,
   INPUT = 2,
   OUTPUT = 3
} PortIndex;

typedef struct
{
   // Port buffers
   const float *gain;
   const float *threshold;
   const float *input;
   float *output;
   float rising;
   float lastOutputValue;
   float lastInputValue;
   float lastWaveLoudness;
   double thisWaveLoudness;
   unsigned long samplesSinceLastWave;
   float rateAndGainCompensation;
} GuitarSynthState;

static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
            double rate,
            const char *bundle_path,
            const LV2_Feature *const *features)
{
   GuitarSynthState *guitarSynthState = (GuitarSynthState *)calloc(1, sizeof(GuitarSynthState));

   guitarSynthState->rising = 0.0f;
   guitarSynthState->lastOutputValue = 0.0f;
   guitarSynthState->lastWaveLoudness = 0.0f;
   guitarSynthState->thisWaveLoudness = 0.0f;
   guitarSynthState->samplesSinceLastWave = 0;
   guitarSynthState->rateAndGainCompensation = 0.1f * (48000.0f / (float)rate);

   return (LV2_Handle)guitarSynthState;
}

static void connect_port(LV2_Handle instance, uint32_t port, void *data)
{
   GuitarSynthState *guitarSynthState = (GuitarSynthState *)instance;

   switch ((PortIndex)port)
   {
      case GAIN:
         guitarSynthState->gain = (const float *)data;
         break;
      case THRESHOLD:
         guitarSynthState->threshold = (const float *)data;
         break;
      case INPUT:
         guitarSynthState->input = (const float *)data;
         break;
      case OUTPUT:
         guitarSynthState->output = (float *)data;
         break;
   }
}

static void activate(LV2_Handle instance)
{
  GuitarSynthState *guitarSynthState = (GuitarSynthState *)instance;
}

static float calcMultiplicator(GuitarSynthState *guitarSynthState);

static void run(LV2_Handle instance, uint32_t n_samples)
{
   GuitarSynthState *guitarSynthState = (GuitarSynthState *)instance;

   const float *const input = guitarSynthState->input;
   float *const output = guitarSynthState->output;

   // TODO: add square wave function output based on a new mode. This should also be based on input loudness.
   
   // TODO: Maybe add a second mode for describing how loudness is calculated. Use some form of RMS?
   //   Do this based on a new mode. That determines wether to do this.
   //   If this is not done, the output will be constant maximum noise. This is basically how VCOs work anyhow.

   float multiplicator = calcMultiplicator(guitarSynthState);
   
   // kind of saw-tooth for upper/lower
   for (uint32_t pos = 0; pos < n_samples; pos++)
   {
      guitarSynthState->rising = input[pos] >= 0.0 ? multiplicator : (-multiplicator);
      
      float absInputValue = 
         input[pos] >= 0.0f 
            ? input[pos]
            : input[pos] * -1.0f;
      
      if ((guitarSynthState->lastInputValue >= 0.0 && guitarSynthState->rising <= 0.0) ||
      (guitarSynthState->lastInputValue <= 0.0 && guitarSynthState->rising >= 0.0))
      {
         guitarSynthState->lastOutputValue = 0.0f;
         
         // NOTE: calculation of loudness based on average
         guitarSynthState->lastWaveLoudness = guitarSynthState->thisWaveLoudness / (double)(guitarSynthState->samplesSinceLastWave + 1);
         
         // NOTE: calculation of loudness based on max value instead of average
         // guitarSynthState->lastWaveLoudness = guitarSynthState->thisWaveLoudness;
         
         guitarSynthState->thisWaveLoudness = 0.0f;
         guitarSynthState->samplesSinceLastWave = 0;
         
         multiplicator = calcMultiplicator(guitarSynthState);
      }
      else
      {
         guitarSynthState->samplesSinceLastWave++;

         if (fabs(guitarSynthState->lastOutputValue) <  2.0f * fmax(guitarSynthState->thisWaveLoudness, guitarSynthState->lastWaveLoudness))
         {
            guitarSynthState->lastOutputValue = guitarSynthState->lastOutputValue + guitarSynthState->rising;
         }
      }
      
      // NOTE: calculation of loudness based on max value instead of average
      // guitarSynthState->thisWaveLoudness = 
      //    absInputValue > guitarSynthState->thisWaveLoudness
      //       ? absInputValue 
      //       : guitarSynthState->thisWaveLoudness;
      
      // NOTE: calculation of loudness based on average
      guitarSynthState->thisWaveLoudness = guitarSynthState->thisWaveLoudness + (double)absInputValue;

      output[pos] = guitarSynthState->lastOutputValue;
      guitarSynthState->lastInputValue = input[pos];
   }
}

float calcMultiplicator(GuitarSynthState *guitarSynthState)
{
   // TODO: remove threshold if not used.
   // if (guitarSynthState->lastWaveLoudness < (*guitarSynthState->threshold) &&
   //     guitarSynthState->lastWaveLoudness > -(*guitarSynthState->threshold))
   // {
   //    return 0.0f;
   // }

   return *(guitarSynthState->gain) * 
      guitarSynthState->lastWaveLoudness * guitarSynthState->rateAndGainCompensation;
}

static void deactivate(LV2_Handle instance)
{
}

static void
cleanup(LV2_Handle instance)
{
   free(instance);
}

static const void *
extension_data(const char *uri)
{
   return NULL;
}

static const LV2_Descriptor descriptor = {URI,
   instantiate,
   connect_port,
   activate,
   run,
   deactivate,
   cleanup,
   extension_data};

LV2_SYMBOL_EXPORT const LV2_Descriptor * lv2_descriptor(uint32_t index)
{
   return index == 0 ? &descriptor : NULL;
}
