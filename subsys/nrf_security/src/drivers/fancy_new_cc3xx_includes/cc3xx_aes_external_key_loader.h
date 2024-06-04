#include "cc3xx_aes.h"

static inline cc3xx_err_t check_key_lock(cc3xx_aes_key_id_t key_id){
        (void) key_id;
        return CC3XX_ERR_SUCCESS;
}

static inline cc3xx_err_t set_key(cc3xx_aes_key_id_t key_id, const uint32_t *key,
                           cc3xx_aes_keysize_t key_size, bool is_tun1)
{
    cc3xx_err_t err = CC3XX_ERR_SUCCESS;
    volatile uint32_t *hw_key_buf_ptr;

#if !defined(CC3XX_CONFIG_AES_CCM_ENABLE) || !defined(CC3XX_CONFIG_AES_TUNNELLING_ENABLE)
    if (is_tun1) {
        assert(0); /* Wrong programming for this driver configuration */
        FATAL_ERR(CC3XX_ERR_INVALID_STATE);
        return CC3XX_ERR_INVALID_STATE;
    }
#endif /* defined(CC3XX_CONFIG_AES_CCM_ENABLE) && defined(CC3XX_CONFIG_AES_TUNNELLING_ENABLE) */

    if (!is_tun1) {
        hw_key_buf_ptr = P_CC3XX->aes.aes_key_0;
    } else {
        hw_key_buf_ptr = P_CC3XX->aes.aes_key_1;
    }

    /* Set key0 size */
    if (!is_tun1) {
        P_CC3XX->aes.aes_control &= ~(0b11U << 12);
        P_CC3XX->aes.aes_control |= (key_size & 0b11U) << 12;
    } else {
        /* Set key1 size */
        P_CC3XX->aes.aes_control &= ~(0b11U << 14);
        P_CC3XX->aes.aes_control |= (key_size & 0b11U) << 14;
    }

    if (key_id == CC3XX_AES_KEY_ID_USER_KEY) {
#ifdef CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE
        size_t key_word_size = 4 + (key_size * 2);

        cc3xx_dpa_hardened_word_copy(hw_key_buf_ptr, key, key_word_size - 1);
        hw_key_buf_ptr[key_word_size - 1] = key[key_word_size - 1];
#else
        hw_key_buf_ptr[0] = key[0];
        hw_key_buf_ptr[1] = key[1];
        hw_key_buf_ptr[2] = key[2];
        hw_key_buf_ptr[3] = key[3];
        if (key_size > CC3XX_AES_KEYSIZE_128) {
            hw_key_buf_ptr[4] = key[4];
            hw_key_buf_ptr[5] = key[5];
        }
        if (key_size > CC3XX_AES_KEYSIZE_192) {
            hw_key_buf_ptr[6] = key[6];
            hw_key_buf_ptr[7] = key[7];
        }
#endif /* CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE */
    }

    return CC3XX_ERR_SUCCESS;
}