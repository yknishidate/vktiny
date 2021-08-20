
#include "vktiny/Vulkan/AccelStruct.hpp"
#include "vktiny/Math.hpp"
#include "vktiny/Log.hpp"

vk::TransformMatrixKHR toVkMatrix(const glm::mat4& transformMatrix)
{
    const glm::mat4 transposedMatrix = glm::transpose(transformMatrix);
    std::array<std::array<float, 4>, 3> data;
    std::memcpy(&data, &transposedMatrix, sizeof(vk::TransformMatrixKHR));
    return vk::TransformMatrixKHR(data);
}

void vkt::BottomLevelAccelStruct::initialize(const Context& context,
                                             const std::vector<Vertex>& vertices,
                                             const Buffer& vertexBuffer,
                                             const std::vector<Index>& indices,
                                             const Buffer& indexBuffer)
{
    AccelStruct::initialize(context);

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

    uint32_t primitiveCount = indices.size() / 3;
    AccelStruct::build(geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, primitiveCount);
}

void vkt::BottomLevelAccelStruct::initialize(const Context& context, const Mesh& mesh)
{
    initialize(context,
               mesh.getVertices(), mesh.getVertexBuffer(),
               mesh.getIndices(), mesh.getIndexBuffer());
}

void vkt::TopLevelAccelStruct::initialize(const Context& context, const BottomLevelAccelStruct& bottomLevelAS, const glm::mat4& transform)
{
    AccelStruct::initialize(context);

    vk::AccelerationStructureInstanceKHR instance{};
    instance.setTransform(toVkMatrix(transform));
    instance.setMask(0xFF);
    instance.setInstanceShaderBindingTableRecordOffset(0);
    instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
    instance.setAccelerationStructureReference(bottomLevelAS.getDeviceAddress());
    instances.push_back(instance);

    instancesSize = sizeof(vk::AccelerationStructureInstanceKHR) * instances.size();
    instancesBuffer.initialize(context,
                               instancesSize,
                               vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
                               vkBU::eShaderDeviceAddress,
                               vkMP::eHostVisible | vkMP::eHostCoherent,
                               instances.data());

    instancesData.setArrayOfPointers(false);
    instancesData.setData(instancesBuffer.getDeviceAddress());

    geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
    geometry.setGeometry({ instancesData });
    geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

    AccelStruct::build(geometry, vk::AccelerationStructureTypeKHR::eTopLevel, instances.size());
}

void vkt::TopLevelAccelStruct::initialize(const Context& context,
                                          const std::vector<BottomLevelAccelStruct>& bottomLevelASs)
{
    AccelStruct::initialize(context);

    vk::TransformMatrixKHR transform = toVkMatrix(flipY(glm::mat4(1.0)));

    for (int i = 0; i < bottomLevelASs.size(); i++) {
        const auto& blas = bottomLevelASs[i];
        vk::AccelerationStructureInstanceKHR instance{};
        instance.setTransform(transform);
        instance.setInstanceCustomIndex(i);
        instance.setMask(0xFF);
        instance.setInstanceShaderBindingTableRecordOffset(0);
        instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
        instance.setAccelerationStructureReference(blas.getDeviceAddress());
        instances.push_back(instance);
    }

    instancesSize = vk::DeviceSize{ sizeof(vk::AccelerationStructureInstanceKHR) * instances.size() };
    instancesBuffer.initialize(context,
                               instancesSize,
                               vkBU::eAccelerationStructureBuildInputReadOnlyKHR | vkBU::eShaderDeviceAddress,
                               vkMP::eHostVisible | vkMP::eHostCoherent,
                               instances.data());

    instancesData.setArrayOfPointers(false);
    instancesData.setData(instancesBuffer.getDeviceAddress());

    geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
    geometry.setGeometry({ instancesData });
    geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

    AccelStruct::build(geometry, vk::AccelerationStructureTypeKHR::eTopLevel, instances.size());
}
