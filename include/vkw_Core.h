#pragma once
#include "vkw_Include.h"
namespace vkw {
	class Window {
	public:
		Window() = default;
		virtual VULKAN_WRAPPER_API ~Window();

		virtual void createSurface(VkInstance instance, VkSurfaceKHR * surface) const = 0;
		virtual void getWindowSize(int * width, int * height) const = 0;

		VULKAN_WRAPPER_API operator void* ();
	protected:
		void * window; 
	};



	struct QueueFamilyTypes {
		std::vector<uint32_t> graphicFamilies;
		std::vector<uint32_t> computeFamilies;
		std::vector<uint32_t> transferFamilies;
		std::vector<uint32_t> sparseBindingFamilies;
		std::vector<uint32_t> protectedFamilies;
	};


	struct PhysicalDevice {
		PhysicalDevice() = default;
		PhysicalDevice(VkPhysicalDevice gpu,
			const std::vector<VkQueueFamilyProperties>  & queueFamilies,
			const VkPhysicalDeviceProperties & props,
			const VkPhysicalDeviceFeatures & featr,
			const VkPhysicalDeviceMemoryProperties & memProp,
			const QueueFamilyTypes & famTypes
		);

		VkPhysicalDevice physicalDevice;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		QueueFamilyTypes queueFamilyTypes;

		VULKAN_WRAPPER_API operator VkPhysicalDevice () const;

		// TODO: these all have to be implemented
		// DOC: check** returns allo chosen and available options, takes in desiredOptions, outs all desired and missingProperties
		VULKAN_WRAPPER_API VkPhysicalDeviceProperties checkProperties(const VkPhysicalDeviceProperties & desiredProperties, VkPhysicalDeviceProperties * outMissingProperties = nullptr) const;
		VULKAN_WRAPPER_API VkPhysicalDeviceFeatures checkFeatures(const VkPhysicalDeviceFeatures & desiredFeatures, VkPhysicalDeviceFeatures * outMissingFeatures = nullptr) const ;
		VULKAN_WRAPPER_API VkPhysicalDeviceMemoryProperties checkMemoryProperties(const VkPhysicalDeviceMemoryProperties & desiredMemoryProperties, VkPhysicalDeviceMemoryProperties * outMissingMemoryProperties = nullptr) const;
	};




	class Instance : public impl::CoreEntity<impl::VkwInstance>{
	public:
		struct CreateInfo {
			std::vector<const char*> desiredExtensions;
			std::vector<const char*> desiredLayers;
			const char* applicationName;
			uint32_t applicationVersion;
		};

		VULKAN_WRAPPER_API Instance();
		VULKAN_WRAPPER_API Instance(const CreateInfo & createInfo);
		VULKAN_WRAPPER_API ~Instance();

		const std::vector<PhysicalDevice> & physicalDevices;

		VULKAN_WRAPPER_API void createInstance(const CreateInfo & createInfo);

		static VULKAN_WRAPPER_API std::vector<const char*> checkExtensions(const std::vector<const char*> & desiredExtensions, std::vector<const char*> * outMissingExtensions = nullptr);
		static VULKAN_WRAPPER_API std::vector<const char*> checkLayers(const std::vector<const char*> & desiredLayers, std::vector<const char*> * outMissingLayers = nullptr);
	private:
		std::vector<PhysicalDevice> physicalDevices_m;
	};




	class Surface : public impl::CoreEntity<impl::VkwSurfaceKHR> {
	public:
		struct CreateInfo {
			const Window & window; 
		};

		VULKAN_WRAPPER_API Surface() = default;
		VULKAN_WRAPPER_API Surface(const CreateInfo & createInfo);
		VULKAN_WRAPPER_API Surface(const Window & window);

		VULKAN_WRAPPER_API void createSurface(const CreateInfo & createInfo);
		VULKAN_WRAPPER_API void createSurface(const Window & window);

		std::vector<VkSurfaceFormatKHR> availableFomats(VkPhysicalDevice gpu) const;
		std::vector<VkPresentModeKHR> availablePresentModes(VkPhysicalDevice gpu) const;
		VkSurfaceCapabilitiesKHR capabilities(VkPhysicalDevice gpu) const;
		VkExtent2D extent(VkPhysicalDevice gpu) const;
	private:
		const Window * window;
	};




	class Device : public impl::CoreEntity<impl::VkwDevice>{
	public:
		// add globalSystemPrority 
		struct AdditionalQueueCreateInfo {
			uint32_t family = std::numeric_limits<uint32_t>::max();
			uint32_t index = std::numeric_limits<uint32_t>::max();
			std::vector<float> priorities;
			VkDeviceQueueCreateFlags flags = 0;
		};

		struct PreSetQueueCreateInfo {
			uint32_t family = std::numeric_limits<uint32_t>::max();
			uint32_t index = std::numeric_limits<uint32_t>::max();
			float priority = 1.0;
			VkDeviceQueueCreateFlags flags = 0;
		};

		struct QueueInfo {
			VkQueue queue = VK_NULL_HANDLE;
			int family = -1;
			int index = -1;
			float priority;

			VULKAN_WRAPPER_API operator VkQueue () const;
		};

		struct PreSetQueuesCreateInfo {
			bool createPresent = true;
			PreSetQueueCreateInfo present;
			bool createGraphics = true;
			PreSetQueueCreateInfo graphics;
			bool createTransfer = true;
			PreSetQueueCreateInfo transfer;
			bool createCompute = true;
			PreSetQueueCreateInfo compute;
		};

		struct CreateInfo {
			PhysicalDevice physicalDevice;	// no multi gpu support available
			PreSetQueuesCreateInfo preSetQueues;
			std::vector<AdditionalQueueCreateInfo> additionalQueues;
			VkPhysicalDeviceFeatures features;
			std::vector<const char*> extensions;
			std::vector<VkSurfaceKHR> surfaces;
		};

		VULKAN_WRAPPER_API Device();
		VULKAN_WRAPPER_API Device(const CreateInfo & createInfo);
	
		const QueueInfo & graphicsQueue;
		const QueueInfo & transferQueue;
		const QueueInfo & presentQueue;
		const QueueInfo & computeQueue;
		const std::vector<QueueInfo> & additionalQueues;

		VULKAN_WRAPPER_API void createDevice(const CreateInfo & createInfo);
	private:
		impl::Registry * deviceRegistry;
		QueueInfo graphicsQueue_m;
		QueueInfo transferQueue_m;
		QueueInfo presentQueue_m;
		QueueInfo computeQueue_m;
		std::vector<QueueInfo> additionalQueues_m;

		void createRegistry(const CreateInfo & createInfo);
		std::vector<const char*> setupExtensions(const std::vector<const char*> & extensions);
		VkPhysicalDeviceFeatures setupFeatures(const VkPhysicalDeviceFeatures & deviceFeatures);
		std::vector<VkDeviceQueueCreateInfo> setupPresetQueues(const PhysicalDevice & gpu, const PreSetQueuesCreateInfo & presetQueues, std::map<int, std::vector<float>> & priorities, const std::vector<VkSurfaceKHR> & surfaces);
		std::vector<VkDeviceQueueCreateInfo> setupQueueCreation(const CreateInfo & info, std::map<int, std::vector<float>> & priorities);
	};
}

