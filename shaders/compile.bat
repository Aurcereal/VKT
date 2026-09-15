slangc material/pbr-obj.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/pbr-obj.spv
slangc material/solid-color.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/solid-color.spv
slangc material/pbr-gltf-prim.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/pbr-gltf-prim.spv
slangc material/vbd-shader.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/vbd-shader.spv
slangc material/display-probe-depth-test.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/display-probe-depth-test.spv
slangc material/skybox.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/skybox.spv
slangc material/reflect.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/reflect.spv
slangc material/raytraced-view.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o compiled/raytraced-view.spv

slangc compute/depth-buffer-to-texture.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o compiled/depth-buffer-to-texture.spv
slangc compute/spherical-harmonics-sky.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o compiled/spherical-harmonics-sky.spv
slangc compute/spherical-harmonics-env-prog.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o compiled/spherical-harmonics-env-prog.spv
slangc compute/spherical-harmonics-env-feedback.slang -I "module" -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry main -o compiled/spherical-harmonics-env-feedback.spv