# FCN Lab — build incrementale con GNU make.
.DELETE_ON_ERROR:

CXX := g++
GNPORT_DIR ?= ../VBD2Cpp
MATPLOT_CPPFLAGS ?=
MATPLOT_LDFLAGS ?= -L/usr/local/lib -L/usr/local/lib/Matplot++
MATPLOT_LIBS ?= -lmatplot -lnodesoup -lpng -ltiff -lz -ljpeg -lm

FCN_CPPFLAGS := -I. -I$(GNPORT_DIR)/include $(MATPLOT_CPPFLAGS)
CXXFLAGS ?= -O2
FCN_WARNINGS := -Wall -Wextra -Wpedantic \
                -Wno-unused-parameter -Wno-unused-variable -Wno-unused-local-typedefs \
                -Wno-sign-compare -Wno-unused-function

TARGET := fcn_all
BUILD := .build
FCN_NAMES := fcn_all fcn_lib_calc fcn_lib_form fcn_lib_jl_files
GNPORT_NAMES := gn_terminal_posix gn_idle gn_string gn_menu gn_entry gn_form \
                gn_form_runner gn_fs gn_menu_io gn_menu_runner gn_menu_repository
ALGLIB_NAMES := ap linalg alglibinternal alglibmisc
FCN_OBJS := $(addprefix $(BUILD)/,$(addsuffix .o,$(FCN_NAMES)))
GNPORT_OBJS := $(addprefix $(BUILD)/gnport/,$(addsuffix .o,$(GNPORT_NAMES)))
SVD_OBJS := $(BUILD)/svd_qr.o $(addprefix $(BUILD)/alglib/,$(addsuffix .o,$(ALGLIB_NAMES)))
SVD_LIB := $(BUILD)/libsvdqr.a
DEPS := $(FCN_OBJS:.o=.d) $(GNPORT_OBJS:.o=.d) $(SVD_OBJS:.o=.d)

.PHONY: all run clean
all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(FCN_OBJS) $(GNPORT_OBJS) $(SVD_LIB) Makefile
	$(CXX) $(LDFLAGS) $(MATPLOT_LDFLAGS) -o $@ $(FCN_OBJS) $(GNPORT_OBJS) \
		$(SVD_LIB) -Wl,--start-group $(MATPLOT_LIBS) -Wl,--end-group $(LDLIBS)

$(SVD_LIB): $(SVD_OBJS)
	$(AR) rcs $@ $^

$(BUILD)/%.o: %.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(FCN_CPPFLAGS) $(CXXFLAGS) $(FCN_WARNINGS) -std=c++17 -MMD -MP -c $< -o $@

$(BUILD)/gnport/%.o: $(GNPORT_DIR)/src/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(FCN_CPPFLAGS) $(CXXFLAGS) $(FCN_WARNINGS) -std=c++17 -MMD -MP -c $< -o $@

$(BUILD)/alglib/%.o: alglib/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++17 -MMD -MP -c $< -o $@

clean:
	$(RM) -r $(BUILD) $(TARGET)

-include $(DEPS)
