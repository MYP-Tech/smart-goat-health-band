/*
 * Activity score — MVP-1 uses a naive scaling, MVP-3 will swap this for
 * per-goat baseline learning. Interface stays the same so callers don't change.
 */
#pragma once

int activity_score_compute(float avg_motion_ms2);
