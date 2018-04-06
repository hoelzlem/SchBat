#include "SOC.h"

#include "main.h"

#include <stdint.h>

// LUT for open circuit voltage vs SOC where the index equals the SOC in percent
static const uint16_t lut_ocv_mV[101] = {
    2450, 3004, 3239, 3362, 3447, 3511, 3564, 3608, 3647, 3680, 3709, 3734, 3750, 3760, 3766, 3771, 3776, 3780, 3784, 3787, 3791, 3795, 3800, 3806, 3814, 3822, 3830, 3837, 3844, 3851, 3858, 3866, 3873, 3879, 3885, 3891, 3897, 3903, 3909, 3915, 3921, 3927, 3933, 3938, 3943, 3948, 3953, 3957, 3961, 3965, 3969, 3973, 3976, 3978, 3981, 3983, 3985, 3987, 3989, 3991, 3993, 3995, 3996, 3998, 4000, 4001, 4003, 4005, 4006, 4008, 4010, 4011, 4013, 4015, 4017, 4019, 4021, 4023, 4025, 4027, 4030, 4033, 4035, 4038, 4041, 4045, 4049, 4053, 4059, 4065, 4072, 4080, 4090, 4098, 4106, 4114, 4122, 4131, 4141, 4152, 4175
};

// returns soc in promille
uint16_t get_soc_by_ocv (uint16_t ocv_mV)
{
    uint16_t soc_promille;

    // Search for index
    if (ocv_mV < lut_ocv_mV[0]) {
        // Corner case for first element
        // Clip value
        soc_promille = 0;
    } else if (ocv_mV >= lut_ocv_mV[100]) {
        // Corner case for last element
        soc_promille = 1000;
    } else {
        // Linear search for indices and interpolation
        uint lower_index, higher_index;
        
        for (higher_index = 1; higher_index < 101; higher_index += 1) {
            if (ocv_mV < lut_ocv_mV[higher_index]) {
                lower_index = higher_index - 1;
                break;
            }
        }
        // Interval is set -> use this information to do linear interpolation between the values
        //
        // SOC = m * (U - U_lower) + n
        //
        //          10 promille
        // SOC = ------------------ * (U - U_lower) + SOC_lower
        //       U_higher - U_lower
        //
        // Division error leads to underestimation of SOC which is better than overestimation by rounding
        soc_promille = (10 * (ocv_mV - lut_ocv_mV[lower_index]) ) / (lut_ocv_mV[higher_index] - lut_ocv_mV[lower_index]) + 10 * lower_index;
    }

    return soc_promille;
}
