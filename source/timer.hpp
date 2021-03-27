#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <iostream>
#include <map>

namespace TIMER {
	struct timer_record {
		std::chrono::steady_clock::time_point start_time;
		std::chrono::steady_clock::duration cumu_time = std::chrono::steady_clock::duration::zero();
		bool started = false;
	};

	struct timer {
		bool exist(const std::string & name) {
			return (records.count(name) == 1);
		}
		void del(const std::string & name) {
			records.erase(name);
		}
		void del() {
			records.clear();
		}
		void print() {
			std::chrono::duration<double> diff;
			std::cout << " \n====================\n";
			for (auto & rec : records) {
				std::cout << rec.first << "  ";
				if (rec.second.started) {
					diff = rec.second.cumu_time + (std::chrono::steady_clock::now() - rec.second.start_time);
				} else {
					diff = rec.second.cumu_time;
				}
				std::cout << diff.count() << "s\n";
			}
			std::cout << "====================\n \n";
		}
		int num() {
			return records.size();
		}
		void start(const std::string & name) {
			auto & rec = records[name];
			if (!rec.started) {
				rec.start_time = std::chrono::steady_clock::now();
				rec.started = true;
			}
		}
		void stop(const std::string & name) {
			auto & rec = records[name];
			if (rec.started) {
				rec.cumu_time += (std::chrono::steady_clock::now() - rec.start_time);
				rec.started = false;
			}
		}
		double get(const std::string & name) {
			timer_record * rec;
			std::chrono::duration<double> diff;
			try {
				rec = &records.at(name);
			} catch (std::out_of_range err) {
				return 0;
			}
			if (rec->started) {
				diff = rec->cumu_time + (std::chrono::steady_clock::now() - rec->start_time);
			} else {
				diff = rec->cumu_time;
			}
			return diff.count();
		}
		void zero_out(const std::string & name) {
			auto & rec = records[name];
			rec.started = false;
			rec.cumu_time = std::chrono::steady_clock::duration::zero();
		}
		void zero_out() {
			for (auto & rec : records) {
				rec.second.started = false;
				rec.second.cumu_time = std::chrono::steady_clock::duration::zero();
			}
		}

	private:
		std::map<std::string, timer_record> records;
	};

	inline timer benchmark;
} // namespace TIMER
#endif
