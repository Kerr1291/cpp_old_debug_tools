#include "Evaluations.h"
#include <math.h>
namespace kc
{
  
  const std::string k_session_folder = "/math_test_results/";
  
bool operator < (const Evaluator::Delta& d0, const Evaluator::Delta& d1 )
{
  return d0.Get() < d1.Get();
}
  
typedef std::map<int,Session*>::iterator ses_iter;

#define S_FOR_EACH( iter ) \
  for(ses_iter iter = _sessions.begin(); iter != _sessions.end(); ++iter)
    
typedef std::map<int,unsigned long long>::iterator gt_iter;

#define GT_FOR_EACH( iter ) \
  for(gt_iter iter = _games_hit.begin(); iter != _games_hit.end(); ++iter)
    
#define T_FOR_EACH( iter ) \
  for(gt_iter iter = _game_times.begin(); iter != _game_times.end(); ++iter)
  
Evaluator& GetEvaluator()
{
  static Evaluator ev;
  return ev;
}

struct Session
{
  int _type;
  std::string _name;
  
  int _start_credits;
  int _current_credits;
  unsigned long long _games_played;
  
  int _highest_credits;
  int _start_threshold;
  
  unsigned long long _session_count;
  
  std::map<int,unsigned long long> _games_hit;
  
  int _time_played;
  
  double _best_session_score;
  int    _best_session_id;
  double _worst_session_score;
  int    _worst_session_id;
  
  int    _good_session_count;
  int    _bad_session_count;
  
  int    _num_payouts;
  int    _total_payout;
  int    _total_earnings;
  
  int    _cash_out_loss_threshold;
  
  int    _games_to_skip;
  
  Session(int type, std::string name="")
  :_type(type)
  ,_name(name)
  ,_start_credits(type)
  ,_current_credits(type)
  ,_games_played(0)
  ,_highest_credits(0)
  ,_start_threshold(0)
  ,_session_count(0)
  ,_time_played(0)
  {
    _best_session_score=0;
    _best_session_id=0;
    _worst_session_score=0;
    _worst_session_id=0;
    _good_session_count=0;
    _bad_session_count=0;
    _current = Evaluator::Delta( _start_credits, _start_credits );
    
    _num_payouts=0;
    _total_payout=0;
    _total_earnings=0;
    _games_to_skip=0;
    
    _cash_out_loss_threshold = std::max(_type * 0.5, 500.0);
    
    {//games
      std::stringstream file_path;
      file_path << k_session_folder << "Session_Games_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
    {//_highest_credits
      std::stringstream file_path;
      file_path << k_session_folder << "Session_HighestCredits_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out << _highest_credits <<"\n";
      out.close();
    }
    {//losing streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_LoseStreaks_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out << _highest_credits <<"\n";
      out.close();
    }
    {//winning streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_WinStreaks_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out << _highest_credits <<"\n";
      out.close();
    }
    {//losing streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_LoseStreaks_ALL_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
    {//winning streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_WinStreaks_ALL_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
    
    {//final stats 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_FINAL_STATS_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
    {//winning streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_SCORE_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
    {//_time_played 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_TIME_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
    
    
    {//payouts
      std::stringstream file_path;
      file_path << k_session_folder << "Session_PAYOUT_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
      out.close();
    }
  }
  
  void Reset(int start_credits)
  {
    _time_played = 0;
    _saved_deltas.clear();
    
    if( (start_credits - _type) <= 0 )
    {
      _start_credits = 1000000;
    }
    
    _start_credits = start_credits;
    _games_played = 0;
    _current = Evaluator::Delta( _start_credits, _start_credits );
    _current_credits = _start_credits;
    _highest_credits = 0;
    _session_count++;
    _games_hit.clear();
  }
  
  void HardReset(int start_credits)
  {
    _time_played = 0;
    _saved_deltas.clear();
    _start_credits = start_credits;
    _games_played = 0;
    _current = Evaluator::Delta( _start_credits, _start_credits );
    _current_credits = _start_credits;
    _highest_credits = 0;
    _games_hit.clear();
  }
  
  void Write()
  {
    {//games
      std::stringstream file_path;
      file_path << k_session_folder << "Session_Games_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << _games_played <<"\n";
      out.close();
    }
    
    {//_highest_credits
      std::stringstream file_path;
      file_path << k_session_folder << "Session_HighestCredits_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << _highest_credits+_type <<"\n";
      out.close();
    }
    {//losing streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_LoseStreaks_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << LargestLosingStreak() <<"\n";
      out.close();
    }
    {//winning streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_WinStreaks_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << LargestWinningStreak() <<"\n";
      out.close();
    }
    {//losing streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_LoseStreaks_ALL_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      WriteAllLosingStreaks( out );
      out.close();
    }
    {//winning streaks
      std::stringstream file_path;
      file_path << k_session_folder << "Session_WinStreaks_ALL_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      WriteAllWinningStreaks( out );
      out.close();
    }
    
    {//final stats 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_FINAL_STATS_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << "=====Session: " <<_session_count <<"\n";
      GT_FOR_EACH( iter )
      {
        out << " Game Type = " <<iter->first <<" | Number hit = "<<iter->second <<"\n";
      }
      out.close();
    }
    
    {//score 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_SCORE_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      WriteScore( out );
      out.close();
    }
    
    {//_time_played 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_TIME_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << _time_played <<"\n";
      out.close();
    }
    //WritePayouts();
    
  }
  
  void Record(Evaluator::Delta& delta, int game_type = 0, int game_time=0)
  {
    if( _games_to_skip > 0 )
    {
      _games_to_skip--;
      if( _games_to_skip == 0 )
        HardReset(1000000);
      return;
    }
  
    _games_played++;
    
    _time_played += game_time;
    
    if( _games_hit.count( game_type ) == 0 )
      _games_hit[game_type] = 0;
    
    _games_hit[game_type]++;
    
    _current_credits = delta._c1;
    
    if( _current_credits-_start_credits > _highest_credits )
      _highest_credits = _current_credits-_start_credits;
    
    //_current.Print("current");
    //delta.Print("incoming");
    
    if( _current.Type() == DeltaType::k_new )
    {
      _current.Update( delta.Get() );
      
      if( std::abs( _current.GetStreak() ) < std::abs( _start_threshold ) )
        _current.SetType( DeltaType::k_new );
    }
    else
    {
      Evaluator::Delta new_delta = _current;
      new_delta.Update( delta.Get() );
      
      //new_delta.Print("new_delta");
      
      if( _current.Type() != new_delta.Type() )
      {
        _saved_deltas.push_back( _current );
        _current = delta;
      }
      else
      {
        _current.Update( delta.Get() );
      }
    }
    
    bool force_cashout = false;
    if( _cash_out_loss_threshold > 0 )
    {
      /*
      if( _highest_credits - _start_threshold > 0 
       && (std::abs( LargestLosingStreak() ) >= _cash_out_loss_threshold 
                           || ( _current.Get() < 0
                             && std::abs( _current.Get() ) >= _cash_out_loss_threshold) ) )
      {
        force_cashout = true;
      }
      */
      
      //if( _highest_credits - _start_threshold > 0 && _highest_credits > 1.5 * _type)
      if( _highest_credits > 5000 )
      {
        force_cashout = true;
      }
    }
    
    if( force_cashout ||  _current_credits <= (_start_credits - _type) || _current_credits <= 0 )
    {
      if( _current.Type() == DeltaType::k_new )
        _current.UpdateType();
      _saved_deltas.push_back( _current );
      Write();
      Reset(_current_credits);
    }
  }
  
  void WriteScore(std::fstream& out)
  {
    double score = 0.0;
    
    int bonuses_hit = 0;
    double bonuses_hit_score = 0.0;
    GT_FOR_EACH( iter )
    {
      if( iter->first == 0 )
        continue;
      
      double bonus_score = static_cast<double>(iter->second);
      
      bonuses_hit_score += bonus_score;
      
      bonuses_hit += iter->second;
    }
    
    //NOTE: make these settable parameters
    const double min_good_bonus_rate = 80.0; //# of games on average per bonus to be considered a "good" session
    const double bonus_weight        = 1000.0;
    const double time_weight         = 900.0;
    const double games_weight        = 800.0;
    const double highest_credits_weight = 1500.0;
    const double win_streak_weight   = 0.4;
    const double lose_streak_weight  = 0.65;
    
    bonuses_hit_score = (min_good_bonus_rate * bonuses_hit_score) / static_cast<double>(_games_played);
    
    bonuses_hit_score = log( 0.5 + bonuses_hit_score ) * bonus_weight;
    
    score += bonuses_hit_score;
    
    double seconds_played = _time_played / 1000.0;
    
    double normalized_time = seconds_played / ( static_cast<double>( _type ) * 0.30 );
    
    double time_score = log( normalized_time ) * time_weight;
    
    score += time_score;
    
    score += LargestWinningStreak() * win_streak_weight;
    
    double games_played = _games_played;
    
    double normalized_games = (15.0 * games_played) / static_cast<double>( _type );
    
    double games_played_score = log( normalized_games ) * games_weight;
    
    score += games_played_score;
    
    score += LargestLosingStreak() * lose_streak_weight;
    
    double highest_credits_score = std::max(_highest_credits,0);
    
    highest_credits_score = highest_credits_score / ( static_cast<double>( _type ) * 0.16 );
    
    highest_credits_score = std::max(highest_credits_score,0.4);
    
    highest_credits_score = log( highest_credits_score + 0.2 ) * highest_credits_weight;
    
    score += highest_credits_score;
    
    
    {//session stats 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_BREAKDOWN_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
    
    out<<"======================================================================\n";
    out<<"Scoring session "<<_type <<" with final score "<<score <<"\n";
    out<<"Session: " <<_session_count <<"\n";
    out<<"Score Breakdown.... \n";
    out<<"seconds_played score "<<seconds_played <<" = "<<time_score <<" ( "<<(seconds_played)/60.0 <<"m )" <<" ( "<<(seconds_played)/3600.0 <<"h )" <<"\n";
    out<<"LargestWinningStreak score "<<LargestWinningStreak() <<" = " <<LargestWinningStreak()*win_streak_weight <<"\n";
    out<<"bonuses hit score (#hit = score) "<<bonuses_hit <<" = "<<bonuses_hit_score <<"\n";
    out<<"games_played score (#hit = score) "<<_games_played <<" = " <<(games_played_score) <<"\n";
    out<<"LargestLosingStreak score " <<LargestLosingStreak() <<" = "<<(LargestLosingStreak()*lose_streak_weight) <<"\n";
    out<<"highest_credits_score score "<<_highest_credits <<" = "<<(highest_credits_score) <<"\n";
    out<<"===++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++====\n";
    
     out.close();
    }
    
    out << score <<"\n";
    
    
    if( score < _worst_session_score )
    {
      _worst_session_id = _session_count;
      _worst_session_score = score;
    }
    if( score > _best_session_score )
    {
      _best_session_id = _session_count;
      _best_session_score = score;
    }
    if( score < 0 )
      _bad_session_count++;
    if( score > 0 )
      _good_session_count++;
  }
  
  void WritePayouts()
  {
    int payout = 0;
    payout = (_current_credits-_start_credits);
      
    if( _current_credits-_start_credits > 0 )
    {
      _num_payouts++;
      _total_payout+=(_current_credits-_start_credits);
      
      if(payout > 500)
        _games_to_skip = 20;
      if(payout > 1000)
        _games_to_skip = 50;
      if(payout > 2000)
        _games_to_skip = 100;
      if(payout > 5000)
        _games_to_skip = 200;
      if(payout > 10000)
        _games_to_skip = 500;
      _games_to_skip = 10;
    }
    
    _total_earnings+=(_current_credits-_start_credits);
    
    {//session stats 
      std::stringstream file_path;
      file_path << k_session_folder << "Session_PAYOUT_" << _type;
      std::fstream out;
      out.open( file_path.str().c_str(), std::ios::out | std::ios::app );
      out << payout <<"\n";
      out.close();
    }
  }
  
  void WriteSessionTotals(std::fstream& out)
  {
    out<<"======================================================================\n";
    out<<"Session totals "<<_type <<"\n";
    out<<"Session Count: " <<_session_count <<"========\n";
    out<<"Bad Sessions: " <<_bad_session_count <<"\n";
    out<<"Worst Session: " <<_worst_session_score <<"\n";
    out<<"Worst ID: " <<_worst_session_id <<"\n";
    out <<"\n";
    out<<"Good Sessions: " <<_good_session_count <<"\n";
    out<<"Best Session: " <<_best_session_score <<"\n";
    out<<"Best ID: " <<_best_session_id <<"\n";
    out <<"\n";
    out<<"Payouts: " <<_num_payouts <<"\n";
    out<<"Total Payouts: " <<_total_payout <<"\n";
    out<<"Total Earnings: " <<_total_earnings <<"\n";
    out <<"\n";
    
    double good_chance = static_cast<double>(_good_session_count) / static_cast<double>(_bad_session_count+_good_session_count);
    double bad_chance = static_cast<double>(_bad_session_count) / static_cast<double>(_bad_session_count+_good_session_count);
    
    out<<"Chance of a good session: " <<good_chance <<"\n";
    out<<"Chance of a bad session: " <<bad_chance <<"\n";
    out<<"=============+++++++++++++++++++++++++++++++++++++++++++++++==========\n";
  }
  
  void WriteAllLosingStreaks(std::fstream& out)
  {
    for( unsigned i = 0; i < _saved_deltas.size(); ++i )
    {
      if( _saved_deltas[i].Type() != DeltaType::k_loss_streak )
        continue;
      
      out << _saved_deltas[i].GetStreak() <<"\n";
    }
    out<<"\n";
  }
  
  void WriteAllWinningStreaks(std::fstream& out)
  {
    for( unsigned i = 0; i < _saved_deltas.size(); ++i )
    {
      if( _saved_deltas[i].Type() != DeltaType::k_win_streak )
        continue;
      
      out << _saved_deltas[i].GetStreak() <<"\n";
    }
    out<<"\n";
  }
  
  int LargestLosingStreak()
  {
    int s = 0;
    for( unsigned i = 0; i < _saved_deltas.size(); ++i )
    {
      if( _saved_deltas[i].Type() != DeltaType::k_loss_streak )
        continue;
      
      if( _saved_deltas[i]._streak_size < s )
      {
        s = _saved_deltas[i].GetStreak();
      }
    }
    return s;
  }
  
  int LargestWinningStreak()
  {
    int s = 0;
    for( unsigned i = 0; i < _saved_deltas.size(); ++i )
    {
      if( _saved_deltas[i].Type() != DeltaType::k_win_streak )
        continue;
      
      if( _saved_deltas[i]._streak_size > s )
      {
        s = _saved_deltas[i].GetStreak();
      }
    }
    return s;
  }
  
  Evaluator::Delta _current;
  std::vector< Evaluator::Delta > _saved_deltas; 
};


Evaluator::Evaluator()
{
  _games_played = 0;
}

Evaluator::~Evaluator()
{
  S_FOR_EACH( iter )
  {
    iter->second->HardReset(1000000);
  }
  

  {//final stats 
    std::stringstream file_path;
    file_path << k_session_folder << "TOTAL_EVAL_STATS";
    std::fstream out;
    out.open( file_path.str().c_str(), std::ios::out | std::ios::trunc );
    out <<" Games played = " <<_games_played <<"\n";
    GT_FOR_EACH( iter )
    {
      out << " Game Type = " <<iter->first <<" | Number hit = "<<iter->second <<"\n";
    }
    S_FOR_EACH( iter )
    {
      iter->second->WriteSessionTotals( out );
    }
    out.close();
  }
}



void Evaluator::AddSessionType(int session_type, int start_credits, const std::string session_name)
{
  Session* ses = new Session(session_type, session_name);
  ses->Reset( start_credits );
  
  if( _sessions.count( session_type ) )
      delete _sessions[session_type];
  _sessions[session_type] = ses;
}

void Evaluator::AddGameTime(int game_type, int time)
{
  _game_times[game_type] = time;
}

int  Evaluator::GetGameTime(int game_type)
{
  if( _game_times.count( game_type ) == 0 )
    return 3000; //default guess
  return _game_times[game_type];
}

void Evaluator::SetSessionThreshold(int type_id, int threshold)
{
  if( _sessions.count( type_id ) == 0 )
    return;
  
  _sessions[type_id]->_start_threshold = threshold;
}

void Evaluator::PrepGame(int credits)
{  
  if( credits < 1000 )
  {
    this->start_credits = 1000000;
    credits = this->start_credits;
    
    S_FOR_EACH( iter )
    {
      iter->second->HardReset(credits);
    }
  }
  
  _last_game_delta = Delta(credits,credits);
}

void Evaluator::RecordGame(int credits, int game_type)
{
  _games_played++;
  
  int time = GetGameTime(game_type);
  
  _last_game_delta.UpdateTo(credits);
  
  //_last_game_delta.Print("last game delta");
  
  if( _games_hit.count( game_type ) == 0 )
    _games_hit[game_type] = 0;
  
  _games_hit[game_type]++;
  
  S_FOR_EACH( iter )
  {
    iter->second->Record( _last_game_delta, game_type, time );
  }
}

void Evaluator::RecordGameWin(int credits_won, int game_type)
{
  _games_played++;
  
  int time = GetGameTime(game_type);
  
  _last_game_delta.Update(credits_won);
  
  
  if( _games_hit.count( game_type ) == 0 )
    _games_hit[game_type] = 0;
  
  _games_hit[game_type]++;
  
  S_FOR_EACH( iter )
  {
    iter->second->Record( _last_game_delta, game_type, time );
  }
}

void Evaluator::RecordGameWinNoUpdate(int credits_won)
{
  _last_game_delta.Update(credits_won);
}












}