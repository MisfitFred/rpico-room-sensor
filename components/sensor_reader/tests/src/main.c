/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * ztest Unit-Tests für sensor_reader
 *
 * Werden mit Twister auf native_sim ausgeführt (kein Hardware nötig).
 * Aufruf:  west twister --testsuite-root components --platform native_sim
 */

#include <zephyr/ztest.h>

#include "sensor_reader.h"

static struct sensor_reader_dev test_dev;

static void *suite_setup(void)
{
	return NULL;
}

static void before_each(void *fixture)
{
	ARG_UNUSED(fixture);
	memset(&test_dev, 0, sizeof(test_dev));
}

ZTEST_SUITE(sensor_reader, NULL, suite_setup, before_each, NULL, NULL);

ZTEST(sensor_reader, test_init_null_dev)
{
	int ret = sensor_reader_init(NULL, NULL, NULL);

	zassert_equal(ret, -EINVAL, "Erwartet -EINVAL, erhalten: %d", ret);
}

ZTEST(sensor_reader, test_start_null_dev)
{
	int ret = sensor_reader_start(NULL);

	zassert_equal(ret, -EINVAL, "Erwartet -EINVAL, erhalten: %d", ret);
}
