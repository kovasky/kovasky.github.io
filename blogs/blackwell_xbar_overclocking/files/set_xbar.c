#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define NV_IOCTL_MAGIC 'F'
#define NV_IOCTL_BASE 200
#define NV_ESC_REGISTER_FD (NV_IOCTL_BASE + 1)
#define NV_ESC_RM_FREE 0x29
#define NV_ESC_RM_CONTROL 0x2a
#define NV_ESC_RM_ALLOC 0x2b

#define NV01_ROOT 0x00000000U
#define NV01_DEVICE_0 0x00000080U
#define NV20_SUBDEVICE_0 0x00002080U

#define CLK_MEASURE_FREQ 0x20809006U
#define CLK_DOMAINS_GET_CONTROL 0x2080901bU
#define CLK_DOMAINS_SET_CONTROL 0x2080d01cU
#define CLK_DOMAINS_CONTROL_SIZE 0x83cU

#define DOMAIN_HEADER_SIZE 0x3cU
#define DOMAIN_STRIDE 0x40U
#define XBAR_DOMAIN_INDEX 1U
#define XBAR_DOMAIN_BASE (DOMAIN_HEADER_SIZE + XBAR_DOMAIN_INDEX * DOMAIN_STRIDE)
#define FREQ_OFFSET_MODE_FIELD (XBAR_DOMAIN_BASE + 0x08U)
#define FREQ_OFFSET_KHZ_FIELD (XBAR_DOMAIN_BASE + 0x0cU)
#define RAIL0_OFFSET_UV_FIELD (XBAR_DOMAIN_BASE + 0x10U)
#define CONTROLLABLE_DOMAIN_MASK 0x000000ffU
#define XBAR_MEASURE_DOMAIN 2U

typedef uint32_t NvHandle;
typedef struct { int ctl_fd; } nv_ioctl_register_fd_t;

typedef struct {
    NvHandle hRoot;
    NvHandle hObjectParent;
    NvHandle hObjectOld;
    uint32_t status;
} NVOS00_PARAMETERS;

typedef struct {
    NvHandle hRoot;
    NvHandle hObjectParent;
    NvHandle hObjectNew;
    uint32_t hClass;
    uint64_t pAllocParms __attribute__((aligned(8)));
    uint32_t paramsSize;
    uint32_t status;
} NVOS21_PARAMETERS;

typedef struct {
    NvHandle hClient;
    NvHandle hObject;
    uint32_t cmd;
    uint32_t flags;
    uint64_t params __attribute__((aligned(8)));
    uint32_t paramsSize;
    uint32_t status;
} NVOS54_PARAMETERS;

typedef struct {
    uint32_t deviceId;
    NvHandle hClientShare;
    NvHandle hTargetClient;
    NvHandle hTargetDevice;
    uint32_t flags;
    uint64_t vaSpaceSize __attribute__((aligned(8)));
    uint64_t vaStartInternal __attribute__((aligned(8)));
    uint64_t vaLimitInternal __attribute__((aligned(8)));
    uint32_t vaMode;
} NV0080_ALLOC_PARAMETERS;

typedef struct { uint32_t subDeviceId; } NV2080_ALLOC_PARAMETERS;

int main(int argc, char **argv) {
    int32_t req_freq = 0;
    int32_t req_volt = 0;
    int apply = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "reset") == 0) {
            req_freq = 0;
            req_volt = 0;
            apply = 1;
        } else {
            req_freq = (int32_t)strtol(argv[1], NULL, 0);
            if (argc > 2) req_volt = (int32_t)strtol(argv[2], NULL, 0);
            apply = 1;
        }
    }

    int ctl = open("/dev/nvidiactl", O_RDWR | O_CLOEXEC);
    int card = open("/dev/nvidia0", O_RDWR | O_CLOEXEC);
    if (ctl < 0 || card < 0) { perror("open device"); return 1; }

    nv_ioctl_register_fd_t regfd = { .ctl_fd = ctl };
    if (ioctl(card, _IOWR(NV_IOCTL_MAGIC, NV_ESC_REGISTER_FD, nv_ioctl_register_fd_t), &regfd) < 0) {
        perror("register_fd");
        return 1;
    }

    NVOS21_PARAMETERS root = { .hClass = NV01_ROOT };
    if (ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_ALLOC, NVOS21_PARAMETERS), &root) < 0 || root.status != 0) {
        fprintf(stderr, "root alloc failed: 0x%08X\n", root.status);
        return 1;
    }
    NvHandle client = root.hObjectNew;

    NV0080_ALLOC_PARAMETERS dev_params = { .deviceId = 0 };
    NVOS21_PARAMETERS device = { .hRoot = client, .hObjectParent = client, .hClass = NV01_DEVICE_0, .pAllocParms = (uintptr_t)&dev_params, .paramsSize = sizeof(dev_params) };
    if (ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_ALLOC, NVOS21_PARAMETERS), &device) < 0 || device.status != 0) {
        fprintf(stderr, "device alloc failed: 0x%08X\n", device.status);
        return 1;
    }
    NvHandle dev_handle = device.hObjectNew;

    NV2080_ALLOC_PARAMETERS subdev_params = { .subDeviceId = 0 };
    NVOS21_PARAMETERS subdevice = { .hRoot = client, .hObjectParent = dev_handle, .hClass = NV20_SUBDEVICE_0, .pAllocParms = (uintptr_t)&subdev_params, .paramsSize = sizeof(subdev_params) };
    if (ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_ALLOC, NVOS21_PARAMETERS), &subdevice) < 0 || subdevice.status != 0) {
        fprintf(stderr, "subdevice alloc failed: 0x%08X\n", subdevice.status);
        return 1;
    }
    NvHandle subdev_handle = subdevice.hObjectNew;

    uint8_t control[CLK_DOMAINS_CONTROL_SIZE];
    memset(control, 0, sizeof(control));
    memcpy(control + 4, &(uint32_t){ CONTROLLABLE_DOMAIN_MASK }, 4);

    NVOS54_PARAMETERS get_ctrl = { .hClient = client, .hObject = subdev_handle, .cmd = CLK_DOMAINS_GET_CONTROL, .params = (uintptr_t)control, .paramsSize = sizeof(control) };
    ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_CONTROL, NVOS54_PARAMETERS), &get_ctrl);

    int32_t prev_freq, prev_volt;
    memcpy(&prev_freq, control + FREQ_OFFSET_KHZ_FIELD, 4);
    memcpy(&prev_volt, control + RAIL0_OFFSET_UV_FIELD, 4);

    uint32_t meas_params[2] = { XBAR_MEASURE_DOMAIN, 0 };
    NVOS54_PARAMETERS meas = { .hClient = client, .hObject = subdev_handle, .cmd = CLK_MEASURE_FREQ, .params = (uintptr_t)meas_params, .paramsSize = sizeof(meas_params) };
    ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_CONTROL, NVOS54_PARAMETERS), &meas);

    printf("Previous State       : XBAR offset = %+d MHz (%d kHz), Voltage offset = %+d mV (%d uV), Measured Clock = %.2f GHz (%u kHz)\n",
           prev_freq / 1000, prev_freq, prev_volt / 1000, prev_volt, (double)meas_params[1] / 1000000.0, meas_params[1]);

    if (apply) {
        control[FREQ_OFFSET_MODE_FIELD] = 0;
        memcpy(control + FREQ_OFFSET_KHZ_FIELD, &req_freq, 4);
        memcpy(control + RAIL0_OFFSET_UV_FIELD, &req_volt, 4);

        NVOS54_PARAMETERS set_ctrl = { .hClient = client, .hObject = subdev_handle, .cmd = CLK_DOMAINS_SET_CONTROL, .params = (uintptr_t)control, .paramsSize = sizeof(control) };
        if (ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_CONTROL, NVOS54_PARAMETERS), &set_ctrl) < 0 || set_ctrl.status != 0) {
            fprintf(stderr, "Failed to apply control! RM Status = 0x%08X\n", set_ctrl.status);
            close(ctl);
            close(card);
            return 1;
        }

        memset(control, 0, sizeof(control));
        memcpy(control + 4, &(uint32_t){ CONTROLLABLE_DOMAIN_MASK }, 4);
        ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_CONTROL, NVOS54_PARAMETERS), &get_ctrl);

        int32_t post_freq, post_volt;
        memcpy(&post_freq, control + FREQ_OFFSET_KHZ_FIELD, 4);
        memcpy(&post_volt, control + RAIL0_OFFSET_UV_FIELD, 4);

        meas_params[0] = XBAR_MEASURE_DOMAIN;
        meas_params[1] = 0;
        ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_CONTROL, NVOS54_PARAMETERS), &meas);

        printf("Applied (Persistent) : XBAR offset = %+d MHz (%d kHz), Voltage offset = %+d mV (%d uV), Measured Clock = %.2f GHz (%u kHz)\n",
               post_freq / 1000, post_freq, post_volt / 1000, post_volt, (double)meas_params[1] / 1000000.0, meas_params[1]);
    }

    NVOS00_PARAMETERS free_params = { .hRoot = client, .hObjectOld = client };
    ioctl(ctl, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_FREE, NVOS00_PARAMETERS), &free_params);
    close(ctl);
    close(card);
    return 0;
}
