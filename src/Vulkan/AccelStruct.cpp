
#include "vktiny/Vulkan/AccelStruct.hpp"
#include "vktiny/Math.hpp"

vk::DeviceSize vkt::AccelStruct::getSize(vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo,
                                         uint32_t primitiveCount)
{
    auto buildSizes = context->getVkDevice().getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice, geometryInfo, primitiveCount);
    return buildSizes.accelerationStructureSize;
}

void vkt::AccelStruct::createBuffer(vk::DeviceSize size)
{
    buffer.initialize(*context, size,
                      vkBU::eAccelerationStructureStorageKHR | vkBU::eShaderDeviceAddress,
                      vkMP::eDeviceLocal);
}

void vkt::AccelStruct::createAccelStruct(vk::DeviceSize size, vk::AccelerationStructureTypeKHR type)
{
    vk::AccelerationStructureCreateInfoKHR createInfo;
    createInfo.setBuffer(buffer.get());
    createInfo.setSize(size);
    createInfo.setType(type);
    accelStruct = context->getVkDevice().createAccelerationStructureKHRUnique(createInfo);
}

void vkt::AccelStruct::build(vk::CommandBuffer commandBuffer,
                             vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo,
                             vk::DeviceSize size, uint32_t primitiveCount)
{
    Buffer scratchBuffer;
    scratchBuffer.initialize(*context, size,
                             vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                             vkMP::eDeviceLocal);
    geometryInfo.setScratchData(scratchBuffer.getDeviceAddress());
    geometryInfo.setDstAccelerationStructure(*accelStruct);

    vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{ primitiveCount , 0, 0, 0 };

    commandBuffer.buildAccelerationStructuresKHR(geometryInfo, &rangeInfo);
}

void vkt::BottomLevelAccelStruct::initialize(const Context& context,
                                             const std::vector<Vertex>& vertices,
                                             const Buffer& vertexBuffer,
                                             const std::vector<Index>& indices,
                                             const Buffer& indexBuffer)
{
    this->context = &context;

    vk::AccelerationStructureGeometryTrianglesDataKHR triangleData;
    triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
    triangleData.setVertexData(vertexBuffer.getDeviceAddress());
    triangleData.setVertexStride(sizeof(Vertex));
    triangleData.setMaxVertex(vertices.size());
    triangleData.setIndexType(vk::IndexType::eUint32);
    triangleData.setIndexData(indexBuffer.getDeviceAddress());

    vk::AccelerationStructureGeometryKHR geometry;
    geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
    geometry.setGeometry({ triangleData });
    geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

    auto type = vk::AccelerationStructureTypeKHR::eBottomLevel;
    vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
    geometryInfo.setType(type);
    geometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
    geometryInfo.setGeometries(geometry);

    // Create buffer and accel struct
    uint32_t primitiveCount = indices.size() / 3;
    vk::DeviceSize size = getSize(geometryInfo, primitiveCount);
    createBuffer(size);
    createAccelStruct(size, type);

    // Build
    auto cmdBuf = context.getDevice().beginGraphicsCommand();
    build(*cmdBuf, geometryInfo, size, primitiveCount);
    context.getDevice().endGraphicsCommand(*cmdBuf);
}

void vkt::BottomLevelAccelStruct::initialize(const Context& context, const Mesh& mesh)
{
    initialize(context,
               mesh.getVertices(), mesh.getVertexBuffer(),
               mesh.getIndices(), mesh.getIndexBuffer());
}

vk::TransformMatrixKHR toVkMatrix(const glm::mat4& transformMatrix)
{
    const glm::mat4 transposedMatrix = glm::transpose(transformMatrix);
    std::array<std::array<float, 4>, 3> data;
    std::memcpy(&data, &transposedMatrix, sizeof(vk::TransformMatrixKHR));
    return vk::TransformMatrixKHR(data);
}

void vkt::TopLevelAccelStruct::initialize(const Context& context,
                                          const BottomLevelAccelStruct& bottomLevelAS,
                                          const glm::mat4& transform)
{
    this->context = &context;

    vk::AccelerationStructureInstanceKHR asInstance;
    asInstance.setTransform(toVkMatrix(flipY(transform)));
    asInstance.setMask(0xFF);
    asInstance.setAccelerationStructureReference(
        bottomLevelAS.getBuffer().getDeviceAddress());
    asInstance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);

    Buffer instancesBuffer;
    instancesBuffer.initialize(context,
                               sizeof(vk::AccelerationStructureInstanceKHR),
                               vkBU::eAccelerationStructureBuildInputReadOnlyKHR
                               | vkBU::eShaderDeviceAddress,
                               vkMP::eHostVisible | vkMP::eHostCoherent);
    instancesBuffer.copy(&asInstance);

    vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
    instancesData.setArrayOfPointers(false);
    instancesData.setData(instancesBuffer.getDeviceAddress());

    vk::AccelerationStructureGeometryKHR geometry;
    geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
    geometry.setGeometry({ instancesData });
    geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

    auto type = vk::AccelerationStructureTypeKHR::eTopLevel;
    vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
    geometryInfo.setType(type);
    geometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
    geometryInfo.setGeometries(geometry);

    // Create buffer and accel struct
    uint32_t primitiveCount = 1;
    vk::DeviceSize size = getSize(geometryInfo, primitiveCount);
    createBuffer(size);
    createAccelStruct(size, type);

    // Build
    auto cmdBuf = context.getDevice().beginGraphicsCommand();
    build(*cmdBuf, geometryInfo, size, primitiveCount);
    context.getDevice().endGraphicsCommand(*cmdBuf);
}
