#ifndef CLOCKSWEEPCACHE_H
#define CLOCKSWEEPCACHE_H

#include <memory>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <mutex>
#include <functional>

typedef unsigned int uint;

template<class ValueT>
class ClockSweepCache
{
public:
	ClockSweepCache()
	{
		m_JobRunning = true;
		m_Job = std::make_unique<std::thread>(&ClockSweepCache::checkAndEvict, this, std::ref(m_JobRunning));
	}

	~ClockSweepCache()
	{
		m_JobRunning = false;
		if (m_Job && m_Job->joinable()) {
			m_Job->join();
		}
	}

	void setEvictCallback(std::function<void(int, ValueT)> callback) {
		m_EvictCallback = callback;
	}

	[[nodiscard]] std::optional<ValueT> get(int key)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_Map.find(key) != m_Map.end())
		{
			m_Map[key].frequency++;
			return m_Map[key].value;
		} else {
			return std::nullopt;
		}
	}

	void put(int key, ValueT value)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Map[key] = {value, 1};
	}

	void remove(int key)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Map.erase(key);
	}

	std::optional<std::pair<int, ValueT>> evictOne() 
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_Map.empty()) return std::nullopt;

		auto it = m_Map.begin();
		int keyToEvict = it->first;
		ValueT valToEvict = it->second.value;

		for (auto& [key, data] : m_Map) {
			if (data.frequency < m_Map[keyToEvict].frequency) {
				keyToEvict = key;
				valToEvict = data.value;
			}
		}

		m_Map.erase(keyToEvict);
		return std::make_pair(keyToEvict, valToEvict);
	}

private:

	struct Data {
		ValueT value;
		int frequency;
	};

	std::unordered_map<int, Data> m_Map;
	std::mutex m_Mutex;
	bool m_JobRunning;
	std::unique_ptr<std::thread> m_Job;
	std::function<void(int, ValueT)> m_EvictCallback;

	void checkAndEvict(const bool &jobRunning)
	{
		using namespace std::chrono_literals;

		while (true)
		{
			if (!jobRunning) return;

			std::vector<std::pair<int, ValueT>> evictions;
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				std::vector<int> toRemove;
				for (auto &[key, data] : m_Map)
				{
					data.frequency--;

					if (data.frequency <= 0)
					{
						toRemove.emplace_back(key);
					}
				}

				for (int key: toRemove)
				{
					evictions.push_back({key, m_Map[key].value});
					m_Map.erase(key);
				}
			}

			// Call callbacks outside the lock
			for (auto& eviction : evictions) {
				#ifdef LOGGING
				printf("Evicting %d\n", eviction.first);
				#endif

				if (m_EvictCallback) {
					m_EvictCallback(eviction.first, eviction.second);
				}
			}

			std::this_thread::sleep_for(1s);
		}
	}
};

#endif
