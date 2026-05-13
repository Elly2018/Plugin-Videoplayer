#!/bin/bash

cd Unity

bash linux_codegen.sh
bash linux_build.sh
bash linux_example_dependencise.sh
bash linux_example_build.sh