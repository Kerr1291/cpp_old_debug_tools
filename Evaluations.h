#pragma once
#include "helpers.h"
  
namespace kc
{
  
namespace DeltaType
{
  const int k_new = 0;
  const int k_win_streak = 1;
  const int k_loss_streak = 2;
}
  
struct Session;

class Evaluator
{
  friend Evaluator& GetEvaluator();     
  friend struct Session;
  
  struct Delta
  {    
    int _c0;
    int _c1;
    int _type;
    int _streak_size;
    Delta(int c0=0, int c1=0)
    :_c0(c0)
    ,_c1(c1)
    ,_streak_size(0)
    {
      UpdateType();
    }
    
    void Print(const std::string label="")
    {
      std::cerr<<"=== "<<label <<" ===\n";
      std::cerr<<"Delta "<<_type <<"\n";
      std::cerr<<"_c0 = "<<_c0 <<"\n";
      std::cerr<<"_c1 = "<<_c1 <<"\n";
      std::cerr<<"_streak_size = "<<_streak_size <<"\n";
    }
    
    void UpdateType()
    {
      
      if(_c0 == _c1)
      {
        _type = DeltaType::k_new;
      }
      else if(_c1 < _c0)
      {
        if( Type() == DeltaType::k_new )
            _streak_size = Get();
        else
        if( Get() < _streak_size )
          _streak_size = Get();
        
        _type = DeltaType::k_loss_streak;
      }
      else if(_c1 > _c0)
      {
        if( Type() == DeltaType::k_new )
            _streak_size = Get();
        else
        if( Get() > _streak_size )
          _streak_size = Get();
        
        _type = DeltaType::k_win_streak;
      }
    }
    
    void UpdateTo(int c){ 
      _c1 = c; 
      UpdateType();
    }
    
    void Update(int c){ 
      _c1 += c;       
      UpdateType();
    }    
    
    int Get() const{
      return _c1 - _c0;
    }
    
    int GetStreak() const{
      return _streak_size;
    }
    
    bool IsWin() const{
      return( Get() >= 0 );
    }
    
    int Type() const{ return _type; }
    void SetType(int t) { _type = t; }
      
  };
  friend bool operator < (const Evaluator::Delta& d0, const Evaluator::Delta& d1 );

  private:
    
    Evaluator();
    
  public:
    
    ~Evaluator();
    
    void AddSessionType(int session_type, int start_credits, const std::string session_name="");
    void SetSessionThreshold(int type_id, int threshold);
    
    void AddGameTime(int game_type, int time);
    int  GetGameTime(int game_type=0);
    
    void PrepGame(int credits);
    void RecordGame(int credits, int game_type=0);
    void RecordGameWin(int credits_won, int game_type=0);
    void RecordGameWinNoUpdate(int credits_won);
    
    int start_credits;
    
  private:
    
    unsigned long long                    _games_played;
    std::map<int,unsigned long long>      _games_hit;
    std::map<int,unsigned long long>      _game_times;
    std::map<int,Session*> _sessions;
    Delta _last_game_delta;
};

Evaluator& GetEvaluator();


bool operator < (const Evaluator::Delta& d0, const Evaluator::Delta& d1 );

}