# nrf_cc3xx_platform is not a driver in the old APIs. Once ctr_drbg/hmac_drbg
# gets real driver API support then the placement here makes more sense
if(TARGET nrf_cc3xx_platform)
  add_subdirectory(nrf_cc3xx_platform)
endif()

if(CONFIG_BUILD_WITH_TFM)
  # When TF-M is used the Crypto driver is in the TF-M image so we
  # don't need to add the sources from this non-secure image
else()

#  if(CONFIG_PSA_CRYPTO_DRIVER_CC3XX)
#	add_subdirectory(nrf_cc3xx)

  # Note: We cannot have the PSA_CRYPTO_DRIVER_CC3XX enabled since this will
  # give us the old library.

  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fancy_new_cc3xx")
    message(FATAL_ERROR "You need to have a folder called fancy_new_cc3xx in this folder which contains points to the"
                         "folder platform/ext/accelerator/cc312/cc312-rom/ in the TF-M repo. A symlink will work.")
  endif()

  set(CC3XX_TARGET_PREFIX fancy_new_cc3xx_)
  set(CC3XX_PLATFORM_INTERFACE zephyr_interface)

  # Required by the building script in the TF-M repo
  set(CMAKE_BUILD_TYPE release)

  add_subdirectory(fancy_new_cc3xx)

  nrf_security_add_zephyr_options(fancy_new_cc3xx_cc312_rom)
  nrf_security_add_zephyr_options(fancy_new_cc3xx_cc312_rom_psa_driver_api)

  target_include_directories(fancy_new_cc3xx_cc312_rom_psa_driver_api
    PUBLIC
    ./fancy_new_cc3xx_includes
  )

  target_link_libraries(${mbedcrypto_target}
    PUBLIC
    fancy_new_cc3xx_cc312_rom_psa_driver_api
 )



#endif()

  if(CONFIG_PSA_CRYPTO_DRIVER_CRACEN)
	add_subdirectory(cracen)
  endif()
endif()

add_subdirectory(zephyr)

if(CONFIG_PSA_CRYPTO_DRIVER_OBERON OR CONFIG_OBERON_BACKEND)
  add_subdirectory(nrf_oberon)
endif()
