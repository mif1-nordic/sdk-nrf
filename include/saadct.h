#ifndef SAADCT_H
#define SAADCT_H

#include <zephyr/types.h>
#include <zephyr/device.h>
#include <nrfx_saadc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	/** One shot mode - one series measurement. The measurement is stopped automatically. */
	saadct_mode_one_shot,
	/** Continous mode - measurement of the series until @ref saadct_stop is called. */
	saadct_mode_continuous,
} saadct_mode_t;

struct api_saadct_config {
	/** Number of measurements in one series. */
	uint32_t num_of_meas;
	/** Number of SAADC channels. */
	uint8_t num_of_channels;
	/** SAADC resolution bits. */
	nrf_saadc_resolution_t resolution;
	/** SAADC sample rate. */
	uint32_t sample_rate;
	/** Operation mode. See @ref saadct_mode_t. */
	saadct_mode_t mode;
	/** User callback called when a measurement series is completed. */
	void (*user_handler)(void * context);
	/** User context to be used by the user callback. */
	void *user_context;
	/** Channels configuration. */
	nrfx_saadc_channel_t const * channels_config;
};

__subsystem struct saadct_driver_api {
	int (*configure)(const struct device *dev, struct api_saadct_config * cfg);
	int (*start)(const struct device *dev, struct k_mem_slab * slab);
	int (*stop)(const struct device *dev, bool wait_until_series_finished);
	int (*meas_get)(const struct device *dev, uint16_t ** data);
	void (*meas_free)(const struct device *dev, uint16_t *data);
	uint32_t (*meas_pending)(const struct device *dev);
};

/**
 * @brief Configure pulse width measurement parameters.
 *
 * Applies configuration for the pulse width measurement. This function
 * must be called before @ref saadct_start.
 *
 * @param dev Pointer to the device structure.
 * @param cfg Pointer to the configuration structure.
 *
 * @retval 0 If successful.
 * @retval -EINVAL Invalid argument.
 */
__syscall int saadct_configure(const struct device *dev, struct api_saadct_config * cfg);

/**
 * @brief Start pulse width measurement.
 *
 * The @p slab parameter must point to a memory slab created with
 * @ref K_MEM_SLAB_DEFINE. The slab block size must be large enough
 * to store a measurement series defined by @ref num_of_meas.
 *
 * @param dev Pointer to the device structure.
 * @param slab Memory slab used to store measurement results.
 *
 * @retval 0 If successful.
 * @retval -EINVAL Invalid argument.
 */
__syscall int saadct_start(const struct device *dev, struct k_mem_slab * slab);

/**
 * @brief Stop pulse width measurement.
 *
 * The function stops pulse width measurement. The measurement can be
 * stopped immediately, or after finishing the series which size is defined
 * by the @ref num_of_meas, depending on @p wait_until_series_finished parameter.
 * The function is always non-blocking.
 *
 * If @p wait_until_series_finished is set to true, the driver finishes
 * the current measurement series before stopping the measurement.
 * The @ref saadct_meas_get returns -EAGAIN error code while the current measurement
 * series is in progress.
 *
 * If @p wait_until_series_finished is set to false, the measurement is
 * stopped immediately and the currently collected (incomplete) measurement
 * series is discarded and will not be available via @ref saadct_meas_get.
 *
 * @param dev Pointer to the device structure.
 * @param wait_until_series_finished Memory slab for measurement results.
 *
 * @retval 0 If successful.
 * @retval -EINVAL Invalid argument.
 */
__syscall int saadct_stop(const struct device *dev, bool wait_until_series_finished);

/**
 * @brief Get the pulse width measurements.
 *
 * After processing measurements, the @ref saadct_meas_free should be called
 *
 * If the measurement series hasn't finished yet, the function returns -EBUSY
 * error code.
 *
 * The function can be called either during measurement, or when it is finished,
 * e.g. after @ref saadct_stop is called.
 *
 * @param dev Pointer to the device structure.
 * @param data Pointer to the user data block.
 *
 * @retval 0 If successful.
 * @retval -EINVAL Invalid argument.
 * @retval -EAGAIN Measurement is in progress and the current series has not been completed yet.
 * @retval -EIO Measurement is stopped and no measurements to be read.
 */
__syscall int saadct_meas_get(const struct device *dev, uint16_t ** data);

/**
 * @brief Release measurement buffer.
 *
 * Releases the buffer returned by @ref saadct_meas_get so that it can be reused
 * by the driver for future measurements.
 *
 * @param dev Pointer to the device structure.
 * @param data Pointer previously returned by @ref saadct_meas_get.
 *
 * @retval 0 If successful.
 * @retval -EINVAL Invalid argument.
 */
__syscall void saadct_meas_free(const struct device *dev, uint16_t *data);

/**
 * @brief Get number of measurement series available.
 *
 * @param dev Pointer to the device structure.
 *
 * @return Number of measurement series available to read.
 */
__syscall uint32_t saadct_meas_pending(const struct device *dev);

static inline int saadct_configure(const struct device *dev, struct api_saadct_config * cfg)
{
	const struct saadct_driver_api *api =
		(const struct saadct_driver_api *)dev->api;

	return api->configure(dev, cfg);
}

int saadct_start(const struct device *dev, struct k_mem_slab * slab)
{
		const struct saadct_driver_api *api =
		(const struct saadct_driver_api *)dev->api;

	return api->start(dev, slab);
}

int saadct_stop(const struct device *dev, bool wait_until_series_finished)
{
		const struct saadct_driver_api *api =
		(const struct saadct_driver_api *)dev->api;

	return api->stop(dev, wait_until_series_finished);
}
int saadct_meas_get(const struct device *dev, uint16_t ** data)
{
	const struct saadct_driver_api *api =
		(const struct saadct_driver_api *)dev->api;

	return api->meas_get(dev, data);
}

void saadct_meas_free(const struct device *dev, uint16_t *data)
{
	const struct saadct_driver_api *api =
		(const struct saadct_driver_api *)dev->api;

	return api->meas_free(dev, data);
}

uint32_t saadct_meas_pending(const struct device *dev)
{
	const struct saadct_driver_api *api =
		(const struct saadct_driver_api *)dev->api;

	return api->meas_pending(dev);
}

#endif /* SAADCT_H */