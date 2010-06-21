HEADER = <<-END
#ifndef CLUA_EVENTS_H
#define CLUA_H


class CGHost;
class CBaseGame;
class CGamePlayer;

class CLuaEvent {
public:
  virtual std::string GetLuaName() {}
  
};

END

class Event < Struct.new(:name, :comment, :attributes)
  def initialize(name, comment, attributes)
    @name = name
    @comment = comment
    @attributes = attributes.to_a
  end
  
  def collect separator, &block
    lines = []
    @attributes.each{ |att| lines << yield(att) }
    lines.join(separator)
  end
  
  def attribute_defs
    collect("\n") { |att| "  #{att[1]} m_#{att[0]};" }
  end
  
  def getter_defs
    collect("\n") { |att| "  #{att[1]} Get#{att[0]}() { return m_#{att[0]}; }" }
  end
  
  
  def constructor_args
    collect(", ") { |att| "#{att[1]} n_#{att[0]}" }
  end
  
  def constructor_assigns
    collect(", ") { |att| "m_#{att[0]}(n_#{att[0]})" }
  end
  
  def to_s
    return <<-END
/* #{@comment} */
class CLua#{@name}Event : public CLuaEvent {
protected:
#{attribute_defs}
public:
  CLua#{@name}Event(#{constructor_args}) : #{constructor_assigns} {}
  std::string GetLuaName() { return "#{@name}"; }
#{getter_defs}
};
END
  end
end

class Attribute < Struct.new(:name, :type); end
def a(name, type)
  Attribute.new(name, type)
end

events = [
  Event.new("GHostInitalized", "Called when GHost is initialized", [
    a("GHost", "CGHost*")
  ]),
  
  Event.new("GHostShuttingDown", "Called when GHost is shutting down", [
    a("GHost", "CGHost*")
  ]),
  
  # Game
  Event.new("PlayerJoined", "Called when a player has successfully joined a game (includes admin game)", [
    a("Game", "CBaseGame*"),
    a("Player", "CGamePlayer*")
  ]),
  
  Event.new("GamePlayerChat", "Called when a player chats ingame or in a game lobby", [
    a("Game", "CBaseGame*"),
    a("Player", "CGamePlayer*"),
    a("Message", "std::string"),
    a("Ingame", "bool")
  ])
]
puts HEADER
puts events.map(&:to_s).join("\n\n")
puts "#endif"