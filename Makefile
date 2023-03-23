CC ?= gcc

CFLAGS += $(INC_FLAGS) -MMD -MP -Werror -Wunused-function -Wunused-variable

LDLIBS = -lgpiod

TARGET := directc_programmer

SRCS := dputil.c dpuser.c dpcom.c dpalg.c JTAG/dpchain.c JTAG/dpjtag.c SPIFlash/dpS25F.c SPIFlash/dpSPIalg.c SPIFlash/dpSPIprog.c G5Algo/dpG5alg.c
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(dir $(SRCS))
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)