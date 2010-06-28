HEADER = <<-END
#ifndef CLUA_EVENTS_H
#define CLUA_H


class CGHost;
class CBaseGame;
class CGamePlayer;
class CLuaRCClientHandler;

class CLuaEvent {
public:
  virtual std::string GetLuaName() {}
  
};

END

class Event < Struct.new(:name, :comment, :attributes, :methods)
  def initialize(name, comment, attributes, methods = [])
    @name = name
    @comment = comment
    @attributes = attributes.to_a
    @methods = methods
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
  
  def methods
    lines = []
    @methods.each{ |m| lines << "  #{m.code}" }
    lines.join("\n")
  end
  
  def to_s
    return <<-END
/* #{@comment} */
class CLua#{@name}Event : public CLuaEvent {
protected:
#{attribute_defs}
public:
  CLua#{@name}Event(#{constructor_args}) : #{constructor_assigns} {}
#{methods.empty? ? "" : methods + "  \n" }
  std::string GetLuaName() { return "#{@name}"; }
#{getter_defs}
};
END
  end
end

class Attribute < Struct.new(:name, :type); end
class CMethod < Struct.new(:code); end
def a(name, type)
  Attribute.new(name, type)
end

def m(code)
  CMethod.new(code)
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
  ]),
  
  Event.new("GHostUpdate", "Called on every update cycle", [
    a("GHost", "CGHost*")
  ]),
  
  Event.new("RCCommandReceived", "Called when an RC command is received", [
    a("GHost", "CGHost*"),
    a("Client", "CLuaRCClientHandler*"),
    a("Command", "std::string"),
    a("Args", "vector<std::string>")
  ], [
    m("std::string GetArg(int pos) { return m_Args[pos]; }"),
    m("int GetArgCount() { return m_Args.size(); }")
  ])
]
puts HEADER
puts events.map(&:to_s).join("\n\n")
puts "#endif"