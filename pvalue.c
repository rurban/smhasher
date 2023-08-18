// by Jacob Christian Munch-Andersen 
// from https://github.com/NoHatCoder/pValue_for_smhasher
// MIT licensed
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

void *pValueTable[65536];

double
hashPValue (int hashBits, uint64_t hashes, uint64_t collisions,
            bool countPairs, double *expectedOut)
{
  if (collisions == 0)
    {
      return 1.0;
    }
  // With countPairs on each pair of colliding hashes is counted, so for
  // instance 4 identical hashes amount to 6 collisions, as they form 6 pairs.
  // With countPairs off each additional hash of a given values only count
  // once, so 4 identical hashes would only count as 3 collisions.
  double expected;
  double hashesD = (double)hashes;
  double possibilities = ldexp (1.0, hashBits);
  double invPossibilities = 1.0 / possibilities;
  if (countPairs)
    {
      expected = hashesD * (hashesD - 1) * 0.5 * invPossibilities;
    }
  else
    {
      if ((hashesD > possibilities) || true)
        {
          expected = hashesD - possibilities
                     + possibilities
                           * pow (sqrt (1.0 - invPossibilities) / M_E,
                                  hashesD * invPossibilities);
        }
      else
        {
          expected = 0.0;
          uint64_t a = 2;
          do
            {
              double incidentChance
                  = possibilities
                    * pow (sqrt (1.0 - invPossibilities) / M_E,
                           (hashesD - (double)a) * invPossibilities);
              uint64_t dividesByPos = a;
              uint64_t factorialMul = a;
              // Order multiplications so that the product does not over- or
              // under-flow.
              while ((dividesByPos | factorialMul) != 0)
                {
                  while (((incidentChance <= 1.0) || (dividesByPos == 0))
                         && factorialMul != 0)
                    {
                      incidentChance *= (double)(hashes - factorialMul + 1)
                                        / (double)(factorialMul);
                      factorialMul--;
                    }
                  while (((incidentChance >= 1.0) || (factorialMul == 0))
                         && dividesByPos != 0)
                    {
                      incidentChance *= invPossibilities;
                      dividesByPos--;
                    }
                }
              incidentChance *= (double)(a - 1);
              expected += incidentChance;
              if (incidentChance * 1000000000000000.0 <= expected)
                {
                  break;
                  // Stop when additional iterations barely change the value.
                }
              a++;
            }
          while (true);
        }
    }
  if (expectedOut != (double *)0)
    {
      *expectedOut = expected;
    }

  if ((!countPairs) && (hashBits <= 32)
      && (expected * hashesD < 100000000000.0))
    {
      // Use an exact p-value table in cases where the Poisson distribution may
      // be inaccurate, and the processing power required is not too much.
      void *tableEntry = (void *)0;
      bool pleaseFree = false;
      uint64_t a;
      int64_t b;
      for (a = 0; a < 65536; a++)
        {
          // Search for a matching entry in the global table
          if (pValueTable[a] == (void *)0)
            {
              break;
            }
          uint64_t *intEntry = (uint64_t *)(pValueTable[a]);
          if ((intEntry[0] == hashBits) && (intEntry[1] == hashes))
            {
              tableEntry = pValueTable[a];
              break;
            }
        }
      if (tableEntry == (void *)0)
        {
          // Create a missing entry
          uint64_t entries = (uint64_t)(expected * 3.0);
          if (entries < 1000)
            {
              entries = 1000;
            }
          double *probabilityTable = calloc (entries, 8);
          probabilityTable[0] = 1.0;
          int64_t rangeBegin = 0;
          int64_t rangeEnd = 1;
          for (a = 0; a < hashes; a++)
            {
              for (b = rangeEnd; b > rangeBegin; b--)
                {
                  probabilityTable[b]
                      = probabilityTable[b]
                            * (1.0 - invPossibilities * (double)(a - b))
                        + probabilityTable[b - 1] * invPossibilities
                              * (double)(a - b + 1);
                }
              probabilityTable[rangeBegin]
                  = probabilityTable[rangeBegin]
                    * (1.0 - invPossibilities * (double)(a - rangeBegin));
              if (probabilityTable[rangeBegin] < 0.000000000000000001)
                {
                  rangeBegin++;
                }
              if ((probabilityTable[rangeEnd] > 0.000000000000000001)
                  && (rangeEnd < entries - 1))
                {
                  rangeEnd++;
                }
            }
          void *storedTable = malloc (8 * (rangeEnd - rangeBegin + 5));
          uint64_t *storedTableI = (uint64_t *)storedTable;
          double *storedTableD = (double *)storedTable;
          storedTableI[0] = hashBits;
          storedTableI[1] = hashes;
          storedTableI[2] = rangeBegin;
          storedTableI[3] = rangeEnd;
          double pSum = 0.0;
          for (b = rangeEnd; b >= rangeBegin; b--)
            {
              pSum += probabilityTable[b];
              storedTableD[4 + b - rangeBegin] = pSum;
            }
          free (probabilityTable);
          tableEntry = storedTable;
          pleaseFree = true;
          for (a = 0; a < 65536; a++)
            {
              // Search for a matching entry again, another thread may have
              // created it simultaneously. There is a small chance that a race
              // condition will lead to leaking memory.
              if (pValueTable[a] == (void *)0)
                {
                  pValueTable[a] = storedTable;
                  pleaseFree = false;
                  break;
                }
              uint64_t *intEntry = (uint64_t *)(pValueTable[a]);
              if ((intEntry[0] == hashBits) && (intEntry[1] == hashes))
                {
                  tableEntry = pValueTable[a];
                  free (storedTable);
                  pleaseFree = false;
                  break;
                }
            }
        }
      uint64_t *tableEntryI = (uint64_t *)tableEntry;
      double *tableEntryD = (double *)tableEntry;
      double pValue;
      if (collisions > tableEntryI[3])
        {
          pValue = 0.0;
        }
      else if (collisions < tableEntryI[2])
        {
          pValue = 1.0;
        }
      else
        {
          pValue = tableEntryD[4 + collisions - tableEntryI[2]];
        }
      if (pleaseFree)
        {
          free (tableEntry);
        }
      return pValue;
    }
  else
    {
      uint64_t factorialDiv = collisions;
      uint64_t expectedMul = collisions;
      double dividesByED;
      double expFraction = modf (expected, &dividesByED);
      uint64_t dividesByE = (uint64_t)dividesByED;
      double poisson = exp (-expFraction);
      while ((dividesByE | factorialDiv | expectedMul) != 0)
        {
          while (((poisson <= 1.0) || ((factorialDiv | dividesByE) == 0))
                 && expectedMul != 0)
            {
              poisson *= expected;
              expectedMul--;
            }
          while (((poisson >= 1.0) || (expectedMul == 0)) && factorialDiv != 0)
            {
              poisson /= (double)factorialDiv;
              factorialDiv--;
            }
          while (((poisson >= 1.0) || (expectedMul == 0)) && dividesByE != 0)
            {
              poisson *= (1.0 / M_E);
              dividesByE--;
            }
        }
      double pValue;
      if ((double)collisions > expected)
        {
          pValue = poisson;
          uint64_t a = collisions + 1;
          do
            {
              poisson *= expected / (double)a;
              pValue += poisson;
              a++;
            }
          while (!(poisson * 1000000000000000.0 <= pValue));
        }
      else
        {
          double invExpected = 1.0 / expected;
          double invPValue = 0.0;
          uint64_t a = collisions;
          while ((a >= 1) && !(poisson * 1000000000000000.0 <= pValue))
            {
              poisson *= (double)a * invExpected;
              invPValue += poisson;
              a--;
            }
          pValue = 1.0 - invPValue;
        }
      return pValue;
    }
}

double
coinPValue (uint64_t flips, uint64_t heads)
{
  if (flips < 2 * heads)
    {
      heads = flips - heads;
    }
  double p = 1.0;
  uint64_t divide2 = flips;
  uint64_t factorialMul = heads;
  while ((factorialMul != 0) || (divide2 != 0))
    {
      if (((p >= 1.0) || (factorialMul == 0)) && divide2 >= 50)
        {
          p *= 0x1p-50;
          divide2 -= 50;
        }
      if (((p >= 1.0) || (factorialMul == 0)) && divide2 >= 1)
        {
          p *= 0.5;
          divide2--;
        }
      if (((p <= 1.0) || (divide2 == 0)) && factorialMul >= 1)
        {
          p *= (double)(flips - factorialMul + 1) / (double)factorialMul;
          factorialMul--;
        }
    }
  double pValue = p;
  int64_t a;
  for (a = heads; a > 0; a--)
    {
      p *= (double)a / (double)(flips - a + 1);
      pValue += p;
      if (p * 1000000000000000.0 <= pValue)
        {
          break;
        }
    }
  return pValue;
}
