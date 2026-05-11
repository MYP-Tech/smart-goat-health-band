/*
 * MVP-1 activity scoring — naive linear scale.
 * MVP-3 will replace this with per-goat baseline:
 *   - Days 1-5: accumulate hourly mean and stdev into NVS
 *   - Day 6+: compare current vs baseline.mean - 1.5*baseline.stdev
 *   - 3 consecutive low samples → WARNING; 5 + temp>40 → CRITICAL
 */
#include "activity_score.h"

int activity_score_compute(float avg_motion_ms2)
{
    // Empirical scaling: 0–2 m/s² maps to 0–100
    int score = (int)(avg_motion_ms2 * 50.0f);
    if (score < 0)   score = 0;
    if (score > 100) score = 100;
    return score;
}
