#include <node_api.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "opm.h"

#ifdef _WIN32
#include <windows.h>
void sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}
#else
#include <unistd.h>
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

// Global chip instance (one per module instance)
static opm_t* chip = NULL;

// Initialize the OPM chip
napi_value InitChip(napi_env env, napi_callback_info info) {
    napi_status status;
    
    // Allocate and initialize chip if not already done
    if (chip == NULL) {
        chip = (opm_t*)malloc(sizeof(opm_t));
        if (chip == NULL) {
            napi_throw_error(env, NULL, "Failed to allocate OPM chip");
            return NULL;
        }
        memset(chip, 0, sizeof(opm_t));
        OPM_Reset(chip);
    }
    
    napi_value result;
    status = napi_get_undefined(env, &result);
    return result;
}

// Write to OPM register with 10ms delay
napi_value WriteRegister(napi_env env, napi_callback_info info) {
    napi_status status;
    size_t argc = 2;
    napi_value args[2];
    
    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok || argc < 2) {
        napi_throw_error(env, NULL, "Expected 2 arguments: address and data");
        return NULL;
    }
    
    if (chip == NULL) {
        napi_throw_error(env, NULL, "Chip not initialized. Call initChip() first.");
        return NULL;
    }
    
    // Get address
    uint32_t address;
    status = napi_get_value_uint32(env, args[0], &address);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Address must be a number");
        return NULL;
    }
    
    // Get data
    uint32_t data;
    status = napi_get_value_uint32(env, args[1], &data);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Data must be a number");
        return NULL;
    }
    
    // Write address
    OPM_Write(chip, 0, (uint8_t)address);
    
    // Write data
    OPM_Write(chip, 1, (uint8_t)data);
    
    // Wait 10ms after register write (as required)
    sleep_ms(10);
    
    napi_value result;
    status = napi_get_undefined(env, &result);
    return result;
}

// Clock the chip and return samples
napi_value ClockChip(napi_env env, napi_callback_info info) {
    napi_status status;
    
    if (chip == NULL) {
        napi_throw_error(env, NULL, "Chip not initialized. Call initChip() first.");
        return NULL;
    }
    
    int32_t output[2];
    uint8_t sh1, sh2, so;
    
    // Clock the chip to generate one sample
    OPM_Clock(chip, output, &sh1, &sh2, &so);
    
    // Create result array [left, right]
    napi_value result;
    status = napi_create_array_with_length(env, 2, &result);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Failed to create result array");
        return NULL;
    }
    
    napi_value left, right;
    status = napi_create_int32(env, output[0], &left);
    status = napi_create_int32(env, output[1], &right);
    
    status = napi_set_element(env, result, 0, left);
    status = napi_set_element(env, result, 1, right);
    
    return result;
}

// Reset the chip
napi_value ResetChip(napi_env env, napi_callback_info info) {
    napi_status status;
    
    if (chip == NULL) {
        napi_throw_error(env, NULL, "Chip not initialized. Call initChip() first.");
        return NULL;
    }
    
    OPM_Reset(chip);
    sleep_ms(10);
    
    napi_value result;
    status = napi_get_undefined(env, &result);
    return result;
}

// Cleanup
napi_value CleanupChip(napi_env env, napi_callback_info info) {
    if (chip != NULL) {
        free(chip);
        chip = NULL;
    }
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

// Module initialization
napi_value Init(napi_env env, napi_value exports) {
    napi_status status;
    napi_value fn;
    
    // initChip
    status = napi_create_function(env, NULL, 0, InitChip, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "initChip", fn);
    if (status != napi_ok) return NULL;
    
    // writeRegister
    status = napi_create_function(env, NULL, 0, WriteRegister, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "writeRegister", fn);
    if (status != napi_ok) return NULL;
    
    // clockChip
    status = napi_create_function(env, NULL, 0, ClockChip, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "clockChip", fn);
    if (status != napi_ok) return NULL;
    
    // resetChip
    status = napi_create_function(env, NULL, 0, ResetChip, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "resetChip", fn);
    if (status != napi_ok) return NULL;
    
    // cleanupChip
    status = napi_create_function(env, NULL, 0, CleanupChip, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "cleanupChip", fn);
    if (status != napi_ok) return NULL;
    
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
