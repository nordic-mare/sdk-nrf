/*
 * Source material for IPSO Colour object (3335) LwM2M v1.0:
 * https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/version_history/3335-1_0.xml
 */

#define LOG_MODULE_NAME ipso_colour_1_0
#define LOG_LEVEL CONFIG_LWM2M_LOG_LEVEL

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <stdint.h>
#include <init.h>

#include <lwm2m_object.h>
#include <lwm2m_engine.h>
#include <lwm2m_resource_ids.h>

#define IPSO_OBJECT_ID 3335
#define COLOUR_RID 5706

#define MAX_INSTANCE_COUNT CONFIG_LWM2M_IPSO_COLOUR_INSTANCE_COUNT

#define NUMBER_OF_OBJECT_FIELDS 3
#define RESOURCE_INSTANCE_COUNT	NUMBER_OF_OBJECT_FIELDS

#define UNIT_STR_MAX_SIZE	8
#define COLOUR_STR_MAX_SIZE 16
#define APP_TYPE_STR_MAX_SIZE 16

#define SENSOR_NAME "Colour"

static char colour[MAX_INSTANCE_COUNT][COLOUR_STR_MAX_SIZE]; 
static char units[MAX_INSTANCE_COUNT][UNIT_STR_MAX_SIZE];
static char app_type[MAX_INSTANCE_COUNT][APP_TYPE_STR_MAX_SIZE];

static struct lwm2m_engine_obj sensor;
static struct lwm2m_engine_obj_field fields[] = {
    OBJ_FIELD_DATA(COLOUR_RID, R, STRING),
    OBJ_FIELD_DATA(SENSOR_UNITS_RID, R_OPT, STRING),
	OBJ_FIELD_DATA(APPLICATION_TYPE_RID, RW_OPT, STRING)
};

static struct lwm2m_engine_obj_inst inst[MAX_INSTANCE_COUNT];
static struct lwm2m_engine_res res[MAX_INSTANCE_COUNT][NUMBER_OF_OBJECT_FIELDS];
static struct lwm2m_engine_res_inst
		res_inst[MAX_INSTANCE_COUNT][RESOURCE_INSTANCE_COUNT];

static struct lwm2m_engine_obj_inst *colour_create(uint16_t obj_inst_id)
{
	int index, i = 0, j = 0;

	/* Check that there is no other instance with this ID */
	for (index = 0; index < MAX_INSTANCE_COUNT; index++) {
		if (inst[index].obj && inst[index].obj_inst_id == obj_inst_id) {
			LOG_ERR("Can not create instance - "
				"already existing: %u", obj_inst_id);
			return NULL;
		}
	}

	for (index = 0; index < MAX_INSTANCE_COUNT; index++) {
		if (!inst[index].obj) {
			break;
		}
	}

	if (index >= MAX_INSTANCE_COUNT) {
		LOG_ERR("Can not create instance - no more room: %u",
			obj_inst_id);
		return NULL;
	}

	/* Set default values */
	colour[index][0] = '\0';
	units[index][0] = '\0';
	app_type[index][0] = '\0';

	(void)memset(res[index], 0,
		     sizeof(res[index][0]) * ARRAY_SIZE(res[index]));
	init_res_instance(res_inst[index], ARRAY_SIZE(res_inst[index]));

	/* initialize instance resource data */
	INIT_OBJ_RES(COLOUR_RID, res[index], i,
		     res_inst[index], j, 1, false, true,
		     &colour[index], sizeof(*colour),
		     NULL, NULL, NULL, NULL);
	INIT_OBJ_RES_DATA(SENSOR_UNITS_RID, res[index], i, res_inst[index], j,
			  units[index], UNIT_STR_MAX_SIZE);
	INIT_OBJ_RES_DATA(APPLICATION_TYPE_RID, res[index], i, res_inst[index],
			  j, app_type[index], APP_TYPE_STR_MAX_SIZE);

	inst[index].resources = res[index];
	inst[index].resource_count = i;
	LOG_DBG("Created IPSO %s Sensor instance: %d", SENSOR_NAME,
		obj_inst_id);
	return &inst[index];
    
}

static int ipso_colour_init()
{
	sensor.obj_id = 3335;
	sensor.fields = fields;
	sensor.field_count = ARRAY_SIZE(fields);
	sensor.max_instance_count = MAX_INSTANCE_COUNT;
	sensor.create_cb = colour_create;
	lwm2m_register_obj(&sensor);

	return 0;
}

SYS_INIT(ipso_colour_init, APPLICATION,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
