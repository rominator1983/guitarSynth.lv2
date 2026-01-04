#include "lv2/core/lv2.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define URI "http://lv2plug.in/plugins/guitarSynth"

typedef enum
{
   GAIN = 0,
   INPUT = 1,
   OUTPUT = 2
} PortIndex;

typedef struct
{
   // Port buffers
   const float *gain;
   const float *input;
   float *output;
   float rising;
   float lastOutputValue;
   float lastWaveLoudness;
   float thisWaveLoudness;
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

static void run(LV2_Handle instance, uint32_t n_samples)
{
   GuitarSynthState *guitarSynthState = (GuitarSynthState *)instance;

   const float *const input = guitarSynthState->input;
   float *const output = guitarSynthState->output;

   // TODO: magic value. maybe find other range for gain, also this should be dependent on frame rate.
   float multiplicator = *(guitarSynthState->gain) * 0.001f;
   // TODO: This does not work
   //float multiplicator = *(guitarSynthState->gain) * guitarSynthState->lastWaveLoudness;
   
   // TODO: add square wave function output based on a new mode. This should also be based on input loudness.
   
   // TODO: add additional multiplication for the output based on input loudness value.
   //   Consider how the loudness of the input is calculated. At first just use max/min value of period.
   //   Maybe add a second mode for describing how loudness is calculated.
   //   Do this based on a new mode. That determines wether to do this.
   //   If this is not done, the output will be constant maximum noise. This is basically how VCOs work anyhow.

   // kind of saw-tooth for upper/lower
   for (uint32_t pos = 0; pos < n_samples; pos++)
   {
      guitarSynthState->rising = input[pos] >= 0.0 ? multiplicator : -multiplicator;

      if ((guitarSynthState->lastOutputValue >= 0.0 && guitarSynthState->rising < 0.0) ||
          (guitarSynthState->lastOutputValue < 0.0 && guitarSynthState->rising > 0.0))
      {
         guitarSynthState->lastOutputValue = 0.0f;
         guitarSynthState->lastWaveLoudness = guitarSynthState->thisWaveLoudness;
         guitarSynthState->thisWaveLoudness = 0.0;
      }
      else  
      {
         guitarSynthState->lastOutputValue = guitarSynthState->lastOutputValue + guitarSynthState->rising;
         float absInputValue = 
            input[pos] > 0.0f 
               ? input[pos]
               : -input[pos];
         
         guitarSynthState->thisWaveLoudness = 
            absInputValue > guitarSynthState->thisWaveLoudness 
               ? absInputValue 
               : guitarSynthState->thisWaveLoudness;
      }
      
      output[pos] = guitarSynthState->lastOutputValue;
   }
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
