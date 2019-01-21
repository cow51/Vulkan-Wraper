#include "vkw_Operations.h"


namespace vkw {

	/// Command Pool
	CommandPool::CommandPool(const CreateInfo & createInfo) :
		CommandPool(createInfo.queueFamily, createInfo.flags)
	{}

	CommandPool::CommandPool(uint32_t queueFamily, VkCommandPoolCreateFlags flags)
	{
		createCommandPool(queueFamily, flags);
	}

	void CommandPool::createCommandPool(const CreateInfo & createInfo)
	{
		createCommandPool(createInfo.queueFamily, createInfo.flags);
	}

	void CommandPool::createCommandPool(uint32_t queueFamily, VkCommandPoolCreateFlags flags)
	{
		this->queueFamily = queueFamily;
		this->flags = flags;

		VkCommandPoolCreateInfo createInfo = Init::commandPoolCreateInfo();
		createInfo.flags = flags;
		createInfo.queueFamilyIndex = queueFamily;

		vkw::Debug::errorCodeCheck(vkCreateCommandPool(registry.device, &createInfo, nullptr, vkObject), "Failed to create Command Pool");
	}






	/// Command Buffers
	void CommandBuffer::allocateCommandBuffers(std::vector<std::reference_wrapper<CommandBuffer>> commandBuffers, const AllocationInfo & allocInfo)
	{
		CommandBuffer::allocateCommandBuffers(commandBuffers, allocInfo.commandPool, allocInfo.level);
	}

	void CommandBuffer::allocateCommandBuffers(std::vector<CommandBuffer> & commandBuffers, const AllocationInfo & allocInfo)
	{
		CommandBuffer::allocateCommandBuffers(commandBuffers, allocInfo.commandPool, allocInfo.level);
	}

	void CommandBuffer::allocateCommandBuffers(std::vector<CommandBuffer> & commandBuffers, VkCommandPool commandPool, VkCommandBufferLevel level)
	{
		std::vector<VkCommandBuffer> vkCommandBuffers(commandBuffers.size());

		VkCommandBufferAllocateInfo allocInfo = Init::commandBufferAllocateInfo();
		allocInfo.level = level;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (commandBuffers.size()) vkw::Debug::errorCodeCheck(vkAllocateCommandBuffers(commandBuffers[0].registry.device, &allocInfo, vkCommandBuffers.data()), "Failed to allocate ommandBuffers");

		for (uint32_t i = 0; i < commandBuffers.size(); i++) {
			commandBuffers[i].commandPool = commandPool;
			commandBuffers[i].level = level;
			commandBuffers[i].vkObject = vkCommandBuffers[i];
		}
	}

	void CommandBuffer::allocateCommandBuffers(std::vector<std::reference_wrapper<CommandBuffer>> commandBuffers, VkCommandPool commandPool, VkCommandBufferLevel level)
	{
		std::vector<VkCommandBuffer> vkCommandBuffers(commandBuffers.size());

		VkCommandBufferAllocateInfo allocInfo = Init::commandBufferAllocateInfo();
		allocInfo.level = level;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (commandBuffers.size()) vkw::Debug::errorCodeCheck(vkAllocateCommandBuffers(commandBuffers[0].get().registry.device, &allocInfo, vkCommandBuffers.data()), "Failed to allocate ommandBuffers");

		for (uint32_t i = 0; i < commandBuffers.size(); i++) {
			commandBuffers[i].get().commandPool = commandPool;
			commandBuffers[i].get().level = level;
			commandBuffers[i].get().vkObject = vkCommandBuffers[i];
		}
	}

	CommandBuffer::CommandBuffer()
	{
		destructionControl = impl::VKW_DESTR_CONTRL_DO_NOTHING;
	}

	CommandBuffer::CommandBuffer(const AllocationInfo & allocInfo):
		CommandBuffer(allocInfo.commandPool, allocInfo.level)
	{
	}

	CommandBuffer::CommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level) :
		CommandBuffer()
	{
		allocateCommandBuffer(commandPool, level);
	}

	void CommandBuffer::allocateCommandBuffer(const AllocationInfo & allocInfo)
	{
		allocateCommandBuffer(allocInfo.commandPool, allocInfo.level);
	}

	void CommandBuffer::allocateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo allocInfo = Init::commandBufferAllocateInfo();
		allocInfo.level = level;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		vkw::Debug::errorCodeCheck(vkAllocateCommandBuffers(registry.device, &allocInfo, vkObject), "Failed to allocate Command Buffer");
	}

	void CommandBuffer::destroyObject()
	{
		vkObject.destroyObject(destructionControl, [=](VkCommandBuffer obj) {vkFreeCommandBuffers(registry.device, this->commandPool, 1, &obj); });
	}

	void CommandBuffer::freeCommandBuffer()
	{
		destroyObject();
	}

	void CommandBuffer::beginCommandBuffer(VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo * inheritanceInfo)
	{
		VkCommandBufferBeginInfo beginInfo = Init::commandBufferBeginInfo();
		beginInfo.flags = flags;
		beginInfo.pInheritanceInfo = inheritanceInfo;

		vkw::Debug::errorCodeCheck(vkBeginCommandBuffer(*vkObject, &beginInfo), "Failed to start recording of the Command Buffer");
	}

	void CommandBuffer::endCommandBuffer()
	{
		vkw::Debug::errorCodeCheck(vkEndCommandBuffer(*vkObject), "Failed to record command Buffer!");
	}

	void CommandBuffer::resetCommandBuffer(VkCommandBufferResetFlags flags) {
		vkw::Debug::errorCodeCheck(vkResetCommandBuffer(*vkObject, flags), "Failed to reset command Buffer!");
	}

	void CommandBuffer::submitCommandBuffer(VkQueue queue, std::vector<VkSemaphore> semaphore, VkFence fence)
	{
		VkSubmitInfo submitInfo = Init::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = vkObject;
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(semaphore.size());
		submitInfo.pSignalSemaphores = semaphore.data();

		vkw::Debug::errorCodeCheck(vkQueueSubmit(queue, 1, &submitInfo, fence), "Failed to submit Command Buffer");
	}





	/// TranferCimmandPool
	TransferCommandPool::TransferCommandPool(int queueFamilyIndex)
	{
		create(queueFamilyIndex);
	}

	TransferCommandPool::TransferCommandPool(const CreateInfo & createInfo):
		TransferCommandPool()
	{}

	void TransferCommandPool::create(const CreateInfo & createInfo)
	{
		create(createInfo.queueFamilyIndex);
	}

	void TransferCommandPool::create(int queueFamilyIndex)
	{
		VkCommandPoolCreateInfo createInfo = Init::commandPoolCreateInfo();
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex == VKW_DEFAULT_QUEUE ? registry.transferQueue.family : queueFamilyIndex;

		Debug::errorCodeCheck(vkCreateCommandPool(registry.device, &createInfo, nullptr, vkObject), "Failed to create Transfer Command Pool");
	}



	/// GraphicsCommandPool
	GraphicsCommandPool::GraphicsCommandPool(int queueFamilyIndex)
	{
		create(queueFamilyIndex);
	}

	GraphicsCommandPool::GraphicsCommandPool(const CreateInfo & createInfo):
		GraphicsCommandPool(createInfo.queueFamilyIndex)
	{}

	void GraphicsCommandPool::create(const CreateInfo & createInfo)
	{
		create(createInfo.queueFamilyIndex);
	}

	void GraphicsCommandPool::create(int queueFamilyIndex)
	{
		VkCommandPoolCreateInfo createInfo = Init::commandPoolCreateInfo();
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex == VKW_DEFAULT_QUEUE ? registry.graphicsQueue.family : queueFamilyIndex;

		Debug::errorCodeCheck(vkCreateCommandPool(registry.device, &createInfo, nullptr, vkObject), "Failed to create Graphics Command Pool");
	}



	/// ComputeCommandPool
	ComputeCommandPool::ComputeCommandPool(int queueFamilyIndex)
	{
		create(queueFamilyIndex);
	}

	ComputeCommandPool::ComputeCommandPool(const CreateInfo & createInfo):
		ComputeCommandPool(createInfo.queueFamilyIndex)
	{}

	void ComputeCommandPool::create(const CreateInfo & createInfo)
	{
		create(createInfo.queueFamilyIndex);
	}

	void ComputeCommandPool::create(int queueFamilyIndex)
	{
		VkCommandPoolCreateInfo createInfo = Init::commandPoolCreateInfo();
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex == VKW_DEFAULT_QUEUE ? registry.computeQueue.family : queueFamilyIndex;

		Debug::errorCodeCheck(vkCreateCommandPool(registry.device, &createInfo, nullptr, vkObject), "Failed to create Compute Command Pool");
	}

}

