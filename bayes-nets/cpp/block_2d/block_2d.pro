TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    annealing_schedule.cpp \
    block.cpp \
    block_geom.cpp \
    camera.cpp \
    config.cpp \
    driver_aggregator.cpp \
    driver_csv_chain.cpp \
    driver_csv_explr_rate.cpp \
    driver_csv_hidden_rvs.cpp \
    driver_csv_metrics.cpp \
    driver_data_gen.cpp \
    driver_data_image_gen.cpp \
    driver_image_combiner.cpp \
    driver_inference_graddesc.cpp \
    driver_inference_hmc.cpp \
    driver_inference_image_gen.cpp \
    driver_inference_mh.cpp \
    fracture_rvs.cpp \
    hidden_state.cpp \
    initial_block_rvs.cpp \
    metropolis_hastings.cpp \
    prob.cpp \
    sample.cpp \
    sample_vector_adapter.cpp \
    test_archive.cpp \
    test_inference_mh.cpp \
    test_modify_vars.cpp \
    util.cpp

HEADERS += \
    annealing_schedule.hpp \
    block.hpp \
    block_geom.hpp \
    camera.hpp \
    config.hpp \
    fracture_rvs.hpp \
    hidden_state.hpp \
    initial_block_rvs.hpp \
    metropolis_hastings.hpp \
    observed_state.hpp \
    prob.hpp \
    sample.hpp \
    sample_vector_adapter.hpp \
    state.hpp \
    util.hpp

INCLUDEPATH += \
    /home/simon/svn-repos/ivilab/src/lib
