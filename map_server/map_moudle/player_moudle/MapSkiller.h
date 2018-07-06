/*
 * MapSkiller.h
 *
 * Created on: 2013-12-04 14:18
 *     Author: lyz
 */

#ifndef _MAPSKILLER_H_
#define _MAPSKILLER_H_

#include <vector>
#include <map>
namespace google
{
	namespace protobuf
	{
		class Message;
	}
}
using ::google::protobuf::Message;

class Money;
class MapPlayer;
struct FighterSkill;

struct SkillerDetail
{
    typedef std::vector<int> SkillIdList;
    typedef std::vector<SkillIdList> SchemeList;

    SkillIdList nulll_scheme_;

    int current_scheme_;
    SchemeList scheme_list_;

    int rama_skill_;
    std::map<int, int> rama_skill_list_;
};

class MapSkiller
{
public:
    MapSkiller(void);
    virtual ~MapSkiller(void);
    virtual MapPlayer *skill_player(void);

    void reset_map_skiller(void);
    void adjust_map_skiller(void);

    int fetch_skill_list(Message *msg);
    int fetch_noraml_skill_list();
    int fetch_rama_skill_list();
    int fetch_transfer_skill_list();
    int fetch_transfer_skill_list(const std::map<int, int>& skill_map);

    int fetch_skill_scheme(Message *msg);
    int update_skill_scheme(Message *msg);
    int check_passive_skill_pa_event(Message* msg);

    int sync_update_skill(Message *msg);
    int exchange_skill_shortcut(Message *proto);

    int request_skill_level_up(Message *msg);
    int add_used_times_and_level_up(FighterSkill* skill);

    int set_current_rama(Message* msg);
    int set_current_scheme(const int scheme);

    int current_scheme(void);
    SkillerDetail& skiller_detail();
    SkillerDetail::SchemeList& scheme_list(void);
    SkillerDetail::SkillIdList& fetch_cur_scheme_skill();

    int fetch_all_skill_scheme();
    int fetch_skill_shortcut();
    int fetch_skill_special_force();
    int check_passive_skill_levelup(int tips_id, int have_amount);

    bool validate_init_insert_skill(int skill_id);

    int fetch_skill_amount_in_level(const int skill_level);
    int insert_rama_skill(int skill_id, int update = false);
    int insert_skill_shortcut(const int scheme, const int index, const int skill_id);

protected:
    SkillerDetail skiller_detail_;
};

#endif //_MAPSKILLER_H_
