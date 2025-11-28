install(DIRECTORY
        cream-of-the-crop
        DESTINATION "presets"
        COMPONENT Presets
        )

if(ENABLE_TESTING)
    install(DIRECTORY
            tests
            DESTINATION "presets"
            COMPONENT Tests
            )
endif()
