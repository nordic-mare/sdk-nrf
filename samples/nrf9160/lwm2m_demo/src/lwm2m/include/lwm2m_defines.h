#ifndef LWM2M_DEFINES_H__
#define LWM2M_DEFINES_H__

/* LwM2M IPSO Object IDs */
#define IPSO_OBJECT_SECURITY_ID			0
#define IPSO_OBJECT_SERVER_ID			1

#define IPSO_OBJECT_DEVICE_ID			3

#define IPSO_OBJECT_COLOUR_ID 			3335

/* Server RIDs */
#define LIFETIME_RID					1

/* Device RIDs */
#define MANUFACTURER_RID                0
#define MODEL_NUMBER_RID                1
#define SERIAL_NUMBER_RID               2
#define FACTORY_RESET_RID               5
#define POWER_SOURCE_RID                6
#define POWER_SOURCE_VOLTAGE_RID        7
#define POWER_SOURCE_CURRENT_RID        8
#define CURRENT_TIME_RID                13
#define DEVICE_TYPE_RID                 17
#define HARDWARE_VERSION_RID            18
#define BATTERY_STATUS_RID              20
#define MEMORY_TOTAL_RID                21

/* Location RIDs */
#define LATITUDE_RID 					0
#define LONGITUDE_RID 					1
#define ALTITUDE_RID 					2
#define LOCATION_RADIUS_RID 			3
#define LOCATION_VELOCITY_RID			4
#define LOCATION_TIMESTAMP_RID			5
#define LOCATION_SPEED_RID 				6

/* Misc */ 
#define LWM2M_RES_DATA_FLAG_RW			0
#define MAX_LWM2M_PATH_LEN				20

#endif /* LWM2M_DEFINES_H__ */