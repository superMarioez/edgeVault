idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES "sensor_hub" "connectivity" "data_pipeline" driver
)
