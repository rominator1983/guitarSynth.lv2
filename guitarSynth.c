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
   const float *input;
   float *output;
   // State
   int32_t lastZeroCrossing; // offset from end of buffer to last zero-crossing
   int32_t previousLastZeroCrossing; // offset from end of buffer to last zero-crossing
   float last_input;
   float last_output;
   float rate;
   double lastHalfwaveSum;
} GuitarSynthState;

static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
            double rate,
            const char *bundle_path,
            const LV2_Feature *const *features)
{
   GuitarSynthState *guitarSynthState = (GuitarSynthState *)calloc(1, sizeof(GuitarSynthState));
   guitarSynthState->rate = rate;

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

static double calculateLoudness(GuitarSynthState *guitarSynthState, uint32_t pos);

static void resetAfterCrossing(GuitarSynthState *guitarSynthState, uint32_t pos);

static void run(LV2_Handle instance, uint32_t n_samples)
{
   GuitarSynthState *guitarSynthState = (GuitarSynthState *)instance;

   const float *const input = guitarSynthState->input;
   float *const output = guitarSynthState->output;
   
   double halfwaveLoudness;
   double length;

   guitarSynthState->lastZeroCrossing = guitarSynthState->lastZeroCrossing - n_samples;
   guitarSynthState->previousLastZeroCrossing = guitarSynthState->previousLastZeroCrossing - n_samples;

   for (uint32_t pos = 0; pos < n_samples; pos++)
   {
      bool crossed_up = (input[pos] >= 0.0f && guitarSynthState->last_input < 0.0f);
      bool crossed_down = (input[pos] < 0.0f && guitarSynthState->last_input >= 0.0f);

      if (crossed_up)
      {
         halfwaveLoudness = calculateLoudness(guitarSynthState, pos);
         if (guitarSynthState->lastZeroCrossing >= 0)
         {
            // use a minimum of 5 to avoid division by zero.
            length = fmax(5, pos - guitarSynthState->lastZeroCrossing);
            
            for (uint32_t i = guitarSynthState->lastZeroCrossing + 1; i <= pos; i++)
            {
               output[i] = halfwaveLoudness * (double)(length - (i - guitarSynthState->lastZeroCrossing)) / length;
            }
         }
         else
         {
            // use a minimum of 5 to avoid division by zero.
            length = fmax(5, pos);
            halfwaveLoudness = guitarSynthState->last_output;
            
            for (uint32_t i = 0; i <= pos; i++)
            {
               output[i] = halfwaveLoudness * (double)(length - i) / length;
            }
         }
         
         resetAfterCrossing(guitarSynthState, pos);
      }
      else if (crossed_down)
      {
         halfwaveLoudness = calculateLoudness(guitarSynthState, pos);

         if (guitarSynthState->lastZeroCrossing >= 0)
         {
            // use a minimum of 5 to avoid division by zero.
            length = fmax(5, pos - guitarSynthState->lastZeroCrossing);
            
            for (uint32_t i = guitarSynthState->lastZeroCrossing < 0 ? 0 : guitarSynthState->lastZeroCrossing; i <= pos; i++)
            {
               output[i] = halfwaveLoudness * (double)(i - guitarSynthState->lastZeroCrossing) / length;
            }
         }
         else
         {
            // use a minimum of 5 to avoid division by zero.
            length = fmax(5, pos);
            
            for (uint32_t i = 0; i <= pos; i++)
            {
               output[i] =
                  (halfwaveLoudness * (double)(i) / length +
                   guitarSynthState->last_output * (double)(length - i) / length );
            }
         }
         resetAfterCrossing(guitarSynthState, pos);
      }

      guitarSynthState->lastHalfwaveSum += (double)input[pos];
      guitarSynthState->last_input = input[pos];
   }

   // if (guitarSynthState->lastZeroCrossing >= 0)
   // {
      // do prediction for the rest of the buffer (or all of the buffer if there was no crossing).
      // use a minimum of 10 to avoid division by zero.
      length = fmax(10,
         fmax(
         guitarSynthState->lastZeroCrossing - guitarSynthState->previousLastZeroCrossing
         ,n_samples - guitarSynthState->lastZeroCrossing));
      
      halfwaveLoudness = guitarSynthState->lastHalfwaveSum * 2.0 / (double)((int32_t)n_samples - guitarSynthState->lastZeroCrossing);

      for (uint32_t pos = guitarSynthState->lastZeroCrossing < 0 ? 0 : guitarSynthState->lastZeroCrossing; pos < n_samples; pos++)
      {
         if (input[n_samples - 1] >= 0.0f)
            output[pos] = fmax((float)(halfwaveLoudness * (double)(pos - guitarSynthState->lastZeroCrossing) / (double)length), 0.0f);
         else
            output[pos] = fmin((float)(halfwaveLoudness * (double)(length - (pos - guitarSynthState->lastZeroCrossing)) / (double)length), 0.0f);
      }
   // }
   // else
   // {
   //    halfwaveLoudness = calculateLoudness(guitarSynthState, n_samples - 1) / 2.0;
   //    length = n_samples;
            
   //    for (uint32_t i = 0; i < n_samples; i++)
   //    {
   //       output[i] =
   //          halfwaveLoudness * (double)(i) / (length - 1) +
   //          guitarSynthState->last_output * (double)(length - i) / (length - 1);
   //    }
   // }


   // simple gain control with clipping
   for (uint32_t pos = 0; pos < n_samples; pos++)
      output[pos] = fmax(-1.0f, fmin(1.0f, output[pos] * (*guitarSynthState->gain)));

   guitarSynthState->last_output = output[n_samples - 1];
}

void resetAfterCrossing(GuitarSynthState *guitarSynthState, uint32_t pos)
{
   guitarSynthState->previousLastZeroCrossing = guitarSynthState->lastZeroCrossing;
   guitarSynthState->lastZeroCrossing = pos;
   guitarSynthState->lastHalfwaveSum = 0.0;
}

double calculateLoudness(GuitarSynthState *guitarSynthState, uint32_t pos)
{
   // the area of the triangle shall be the same as the area of the halfwave. So the height of the triangle is 2 times the average.
   float loudness = guitarSynthState->lastHalfwaveSum * 2.0 / (double)(pos - guitarSynthState->lastZeroCrossing);

   // clipping does not make sense here.
   // return fmax(-1.0f, fmin(1.0f, loudness));
   return loudness;
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
