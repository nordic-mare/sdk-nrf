#define LOG_MODULE_NAME ipso_colour
#define LOG_LEVEL CONFIG_LWM2M_LOG_LEVEL

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <stdint.h>
#include <init.h>

#include <lwm2m_object.h>
#include <lwm2m_engine.h>
#include <lwm2m_resource_ids.h>

#define COLOUR_VERSION_MAJOR 1

#ifdef CONFIG_LWM2M_IPSO_COLOUR_SENSOR_VERSION_1_1
#define COLOUR_VERSION_MINOR 1
#define COLOUR_MAX_ID 8
#else
#define COLOUR_VERSION_MINOR 0
#define COLOUR_MAX_ID 3
#endif

#define MAX_INSTANCE_COUNT 1
#define UNIT_STR_MAX_SIZE	8
#define COLOUR_STR_MAX_SIZE 11

#define RESOURCE_INSTANCE_COUNT	(COLOUR_MAX_ID)

static char colour[MAX_INSTANCE_COUNT][COLOUR_STR_MAX_SIZE]; 
static char units[MAX_INSTANCE_COUNT][UNIT_STR_MAX_SIZE];

static struct lwm2m_engine_obj colour_sensor;
static struct lwm2m_engine_obj_field fields[] = {
    OBJ_FIELD_DATA(5706, R, STRING),
	OBJ_FIELD_DATA(APPLICATION_TYPE_RID, RW_OPT, STRING),
    OBJ_FIELD_DATA(SENSOR_UNITS_RID, R_OPT, STRING)
};

static struct lwm2m_engine_obj_inst inst[MAX_INSTANCE_COUNT];
static struct lwm2m_engine_res res[MAX_INSTANCE_COUNT][COLOUR_MAX_ID];
static struct lwm2m_engine_res_inst
		res_inst[MAX_INSTANCE_COUNT][RESOURCE_INSTANCE_COUNT];

static struct lwm2m_engine_obj_inst *temp_sensor_create(uint16_t obj_inst_id)
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

	(void)memset(res[index], 0,
		     sizeof(res[index][0]) * ARRAY_SIZE(res[index]));
	init_res_instance(res_inst[index], ARRAY_SIZE(res_inst[index]));

	/* initialize instance resource data */
	INIT_OBJ_RES(5706, res[index], i,
		     res_inst[index], j, 1, false, true,
		     &colour[index], sizeof(*colour),
		     NULL, NULL, NULL, NULL);
	INIT_OBJ_RES_DATA(SENSOR_UNITS_RID, res[index], i, res_inst[index], j,
			  units[index], UNIT_STR_MAX_SIZE);
	INIT_OBJ_RES_OPTDATA(APPLICATION_TYPE_RID, res[index], i,
			     res_inst[index], j);

	inst[index].resources = res[index];
	inst[index].resource_count = i;
	LOG_DBG("Create IPSO Temperature Sensor instance: %d", obj_inst_id);
	return &inst[index];
    
}

int ipso_colour_sensor_init()
{
	colour_sensor.obj_id = 3335;
	colour_sensor.fields = fields;
	colour_sensor.field_count = ARRAY_SIZE(fields);
	colour_sensor.max_instance_count = MAX_INSTANCE_COUNT;
	colour_sensor.create_cb = temp_sensor_create;
	lwm2m_register_obj(&colour_sensor);

	return 0;
}
