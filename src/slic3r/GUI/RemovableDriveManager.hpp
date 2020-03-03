#ifndef slic3r_GUI_RemovableDriveManager_hpp_
#define slic3r_GUI_RemovableDriveManager_hpp_

#include <vector>
#include <string>
#include <functional>

#include <boost/thread.hpp>
#include <tbb/mutex.h>

namespace Slic3r {
namespace GUI {
#if __APPLE__
class RDMMMWrapper;
#endif
    
struct DriveData
{
	std::string name;
	std::string path;
	DriveData(std::string n, std::string p):name(n),path(p){}
};
class RemovableDriveManager
{
#if __APPLE__
friend class RDMMMWrapper;
#endif

public:
	/*
	static RemovableDriveManager& get_instance()
	{
		static RemovableDriveManager instance; 
		return instance;
	}
	RemovableDriveManager(RemovableDriveManager const&) = delete;
	void operator=(RemovableDriveManager const&) = delete;
	*/
	RemovableDriveManager();
	~RemovableDriveManager();
	//call only once. on apple register for unmnount callbacks. Enumerates devices for first time so init shoud be called on linux too.
	void init();
	//update() searches for removable devices, returns false if empty. /time = 0 is forced update, time expects wxGetLocalTime()
	void update(const long time = 0);  
	//triggers callbacks if last used drive was removed
	void check_and_notify();
	bool is_drive_mounted(const std::string &path);
	void eject_drive(const std::string &path);
	//returns path to last drive which was used, if none was used, returns device that was enumerated last
	std::string get_last_save_path() const;
	std::string get_last_save_name() const;
	//returns path to last drive which was used, if none was used, returns empty string
	std::string get_drive_path();
	bool is_path_on_removable_drive(const std::string &path);
	// callback will notify only if device with last save path was removed
	void add_remove_callback(std::function<void()> callback);
	// erases all remove callbacks added by add_remove_callback()
	void erase_callbacks(); 
	//drive_count_changed callback is called on every added or removed device
	void set_drive_count_changed_callback(std::function<void(const bool)> callback);
	//thi serves to set correct value for drive_count_changed callback
	void set_plater_ready_to_slice(bool b);
	// marks one of the eveices in vector as last used
	void set_last_save_path(const std::string &path);
	void verify_last_save_path();
	bool is_last_drive_removed();
	void set_is_writing(const bool b);
	bool get_is_writing() const;
	bool get_did_eject() const;
	void set_did_eject(const bool b);
	std::string get_drive_name(const std::string& path);
	size_t get_drives_count();
	std::string get_ejected_path() const;
	std::string get_ejected_name() const;
private:
	// threaded part
	void thread_proc();
	boost::thread m_thread;
	std::atomic_bool m_thread_enumerate_start;
	std::atomic_bool m_thread_enumerate_finnished;
	tbb::mutex m_drives_mutex;

	std::vector<DriveData> m_current_drives;
	//----------

	void search_for_drives();
	
	//returns drive path (same as path in DriveData) if exists otherwise empty string ""
	std::string get_drive_from_path(const std::string& path);
	void reset_last_save_path();

	bool m_initialized;
	std::vector<std::function<void()>> m_callbacks;
	std::function<void(const bool)> m_drive_count_changed_callback;
	size_t m_drives_count;
	long m_last_update;
	std::string m_last_save_path;
	bool m_last_save_path_verified;
	std::string m_last_save_name;
	bool m_is_writing;//on device
	bool m_did_eject;
	bool m_plater_ready_to_slice;
	std::string m_ejected_path;
	std::string m_ejected_name;
#if _WIN32
	//registers for notifications by creating invisible window
	void register_window();
#else
#if __APPLE__
	std::unique_ptr <RDMMMWrapper> m_rdmmm;
 #endif
    void search_path(const std::string &path, const std::string &parent_path);
    bool compare_filesystem_id(const std::string &path_a, const std::string &path_b);
    void inspect_file(const std::string &path, const std::string &parent_path);
#endif
};
// apple wrapper for RemovableDriveManagerMM which searches for drives and/or ejects them    
#if __APPLE__
class RDMMMWrapper
{
public:
    RDMMMWrapper();
    ~RDMMMWrapper();
    void register_window();
    void list_devices(RemovableDriveManager& parent);
    void eject_device(const std::string &path);
    void log(const std::string &msg);
protected:
    void *m_imp;
    //friend void RemovableDriveManager::inspect_file(const std::string &path, const std::string &parent_path);
};
#endif
}}
#endif

