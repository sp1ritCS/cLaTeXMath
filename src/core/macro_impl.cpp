#include "core/macro_impl.h"

#include "graphic/graphic.h"

using namespace tex;
using namespace std;

namespace tex {

macro(hvspace) {
  size_t i;
  for (i = 0; i < args[1].length() && !isalpha(args[1][i]); i++);
  float f = 0;
  valueof(args[1].substr(0, i), f);

  UnitType unit;
  if (i != args[1].length()) {
    wstring s = args[1].substr(i);
    string str;
    wide2utf8(s.c_str(), str);
    tolower(str);
    unit = SpaceAtom::getUnit(str);
  } else {
    unit = UnitType::point;
  }
  if (unit == UnitType::none) {
    string str;
    wide2utf8(args[1].c_str(), str);
    throw ex_parse("Unknown unit '" + str + "'!");
  }

  return (
    args[0][0] == L'h'
    ? sptr<Atom>(new SpaceAtom(unit, f, 0, 0))
    : sptr<Atom>(new SpaceAtom(unit, 0, f, 0))
  );
}

macro(rule) {
  auto[wu, w] = SpaceAtom::getLength(args[1]);
  auto[hu, h] = SpaceAtom::getLength(args[2]);
  auto[ru, r] = SpaceAtom::getLength(args[3]);

  return sptr<Atom>(new RuleAtom(wu, w, hu, h, ru, -r));
}

macro(cfrac) {
  Alignment numAlign = Alignment::center;
  if (args[3] == L"r") {
    numAlign = Alignment::right;
  } else if (args[3] == L"l") {
    numAlign = Alignment::left;
  }
  Formula num(tp, args[1], false);
  Formula denom(tp, args[2], false);
  if (num._root == nullptr || denom._root == nullptr)
    throw ex_parse("Both numerator and denominator of a fraction can't be empty!");
  sptr<FractionAtom> f(new FractionAtom(num._root, denom._root, true, numAlign, Alignment::center));
  f->_useKern = false;
  f->_type = AtomType::inner;
  auto* r = new RowAtom();
  r->add(sptr<Atom>(new StyleAtom(TexStyle::display, f)));
  return sptr<Atom>(r);
}

macro(sfrac) {
  Formula num(tp, args[1], false);
  Formula den(tp, args[2], false);
  if (num._root == nullptr || den._root == nullptr)
    throw ex_parse("Both numerator and denominator of a fraction can't be empty!");

  float sx = 0.75f, sy = 0.75f, r = 0.45f, sL = -0.13f, sR = -0.065f;
  sptr<Atom> slash = SymbolAtom::get("slash");

  if (!tp.isMathMode()) {
    sx = 0.6f;
    sy = 0.5f;
    r = 0.75f;
    sL = -0.24f;
    sR = -0.24f;
    auto in = sptr<Atom>(new ScaleAtom(SymbolAtom::get("textfractionsolidus"), 1.25f, 0.65f));
    auto* vr = new VRowAtom(in);
    vr->setRaise(UnitType::ex, 0.4f);
    slash = sptr<Atom>(vr);
  }

  auto* snum = new VRowAtom(sptr<Atom>(new ScaleAtom(num._root, sx, sy)));
  snum->setRaise(UnitType::ex, r);
  auto* ra = new RowAtom(sptr<Atom>(snum));
  ra->add(sptr<Atom>(new SpaceAtom(UnitType::em, sL, 0, 0)));
  ra->add(slash);
  ra->add(sptr<Atom>(new SpaceAtom(UnitType::em, sR, 0, 0)));
  ra->add(sptr<Atom>(new ScaleAtom(den._root, sx, sy)));

  return sptr<Atom>(ra);
}

macro(genfrac) {
  sptr<SymbolAtom> L, R;

  Formula left(tp, args[1], false);
  L = dynamic_pointer_cast<SymbolAtom>(left._root);

  Formula right(tp, args[2], false);
  R = dynamic_pointer_cast<SymbolAtom>(right._root);

  bool rule = true;
  auto[unit, value] = SpaceAtom::getLength(args[3]);
  if (args[3].empty()) {
    unit = UnitType::em;
    value = 0.f;
    rule = false;
  }

  int style = 0;
  if (!args[4].empty()) valueof(args[4], style);

  Formula num(tp, args[5], false);
  Formula den(tp, args[6], false);
  if (num._root == nullptr || den._root == nullptr) {
    throw ex_parse("Both numerator and denominator of a fraction can't be empty!");
  }
  sptr<Atom> fa(new FractionAtom(num._root, den._root, rule, unit, value));
  auto* ra = new RowAtom();
  const auto texStyle = static_cast<TexStyle>(style * 2);
  ra->add(sptr<Atom>(new StyleAtom(texStyle, sptr<Atom>(new FencedAtom(fa, L, R)))));

  return sptr<Atom>(ra);
}

macro(overwithdelims) {
  auto num = tp.getFormulaAtom();
  auto den = Formula(tp, tp.getOverArgument(), false)._root;

  if (num == nullptr || den == nullptr)
    throw ex_parse("Both numerator and denominator of a fraction can't be empty!");

  auto left = Formula(tp, args[1], false)._root;
  auto* a = dynamic_cast<BigDelimiterAtom*>(left.get());
  if (a != nullptr) left = a->_delim;

  auto right = Formula(tp, args[2], false)._root;
  a = dynamic_cast<BigDelimiterAtom*>(right.get());
  if (a != nullptr) right = a->_delim;

  auto sl = dynamic_pointer_cast<SymbolAtom>(left);
  auto sr = dynamic_pointer_cast<SymbolAtom>(right);
  if (sl != nullptr && sr != nullptr) {
    sptr<FractionAtom> f(new FractionAtom(num, den, true));
    return sptr<Atom>(new FencedAtom(f, sl, sr));
  }

  auto* ra = new RowAtom();
  ra->add(left);
  ra->add(sptr<Atom>(new FractionAtom(num, den, true)));
  ra->add(right);
  return sptr<Atom>(ra);
}

macro(atopwithdelims) {
  auto num = tp.getFormulaAtom();
  auto den = Formula(tp, tp.getOverArgument(), false)._root;

  if (num == nullptr || den == nullptr)
    throw ex_parse("Both numerator and denominator of a fraction can't be empty!");

  auto left = Formula(tp, args[1], false)._root;
  auto* big = dynamic_cast<BigDelimiterAtom*>(left.get());
  if (big != nullptr) left = big->_delim;

  auto right = Formula(tp, args[2], false)._root;
  big = dynamic_cast<BigDelimiterAtom*>(right.get());
  if (big != nullptr) right = big->_delim;

  auto sl = dynamic_pointer_cast<SymbolAtom>(left);
  auto sr = dynamic_pointer_cast<SymbolAtom>(right);
  if (sl != nullptr && sr != nullptr) {
    sptr<Atom> f(new FractionAtom(num, den, false));
    return sptr<Atom>(new FencedAtom(f, sl, sr));
  }

  auto* ra = new RowAtom();
  ra->add(left);
  ra->add(sptr<Atom>(new FractionAtom(num, den, false)));
  ra->add(right);
  return sptr<Atom>(ra);
}

macro(abovewithdelims) {
  auto num = tp.getFormulaAtom();
  auto[du, dv] = tp.getLength();
  auto den = Formula(tp, tp.getOverArgument(), false)._root;
  if (num == nullptr || den == nullptr) {
    throw ex_parse("Both numerator and denominator of a fraction can't be empty!");
  }

  auto left = Formula(tp, args[1], false)._root;
  auto* big = dynamic_cast<BigDelimiterAtom*>(left.get());
  if (big != nullptr) left = big->_delim;

  auto right = Formula(tp, args[2], false)._root;
  big = dynamic_cast<BigDelimiterAtom*>(right.get());
  if (big != nullptr) right = big->_delim;

  auto sl = dynamic_pointer_cast<SymbolAtom>(left);
  auto sr = dynamic_pointer_cast<SymbolAtom>(right);
  if (sl != nullptr && sr != nullptr) {
    sptr<Atom> f(new FractionAtom(num, den, du, dv));
    return sptr<Atom>(new FencedAtom(f, sl, sr));
  }

  auto* ra = new RowAtom();
  ra->add(left);
  ra->add(sptr<Atom>(new FractionAtom(num, den, true)));
  ra->add(right);
  return sptr<Atom>(ra);
}

macro(textstyles) {
  wstring style(args[0]);
  if (style == L"frak") style = L"mathfrak";
  else if (style == L"Bbb") style = L"mathbb";
  else if (style == L"bold") return sptr<Atom>(new BoldAtom(Formula(tp, args[1], false)._root));
  else if (style == L"cal") style = L"mathcal";

  FontInfos* info = nullptr;
  auto it = Formula::_externalFontMap.find(UnicodeBlock::BASIC_LATIN);
  if (it != Formula::_externalFontMap.end()) {
    info = it->second;
    Formula::_externalFontMap[UnicodeBlock::BASIC_LATIN] = nullptr;
  }
  auto atom = Formula(tp, args[1], false)._root;
  if (info != nullptr) {
    Formula::_externalFontMap[UnicodeBlock::BASIC_LATIN] = info;
  }

  string s;
  wide2utf8(style.c_str(), s);
  return sptr<Atom>(new TextStyleAtom(atom, s));
}

macro(accentbiss) {
  string acc;
  switch (args[0][0]) {
    case '~':
      acc = "tilde";
      break;
    case '\'':
      acc = "acute";
      break;
    case '^':
      acc = "hat";
      break;
    case '\"':
      acc = "ddot";
      break;
    case '`':
      acc = "grave";
      break;
    case '=':
      acc = "bar";
      break;
    case '.':
      acc = "dot";
      break;
    case 'u':
      acc = "breve";
      break;
    case 'v':
      acc = "check";
      break;
    case 'H':
      acc = "doubleacute";
      break;
    case 't':
      acc = "tie";
      break;
    case 'r':
      acc = "mathring";
      break;
    case 'U':
      acc = "cyrbreve";
      break;
  }

  return sptr<Atom>(new AccentedAtom(Formula(tp, args[1], false)._root, acc));
}

macro(left) {
  wstring grep = tp.getGroup(L"\\left", L"\\right");

  auto left = Formula(tp, args[1], false)._root;
  auto* big = dynamic_cast<BigDelimiterAtom*>(left.get());
  if (big != nullptr) left = big->_delim;

  auto right = tp.getArgument();
  big = dynamic_cast<BigDelimiterAtom*>(right.get());
  if (big != nullptr) right = big->_delim;

  auto sl = dynamic_pointer_cast<SymbolAtom>(left);
  auto sr = dynamic_pointer_cast<SymbolAtom>(right);
  if (sl != nullptr && sr != nullptr) {
    Formula tf(tp, grep, false);
    return sptr<Atom>(new FencedAtom(tf._root, sl, tf._middle, sr));
  }

  auto* ra = new RowAtom();
  ra->add(left);
  ra->add(Formula(tp, grep, false)._root);
  ra->add(right);

  return sptr<Atom>(ra);
}

macro(intertext) {
  if (!tp.isArrayMode())
    throw ex_parse("Command \\intertext must used in array environment!");

  wstring str(args[1]);
  replaceall(str, L"^{\\prime}", L"\'");
  replaceall(str, L"^{\\prime\\prime}", L"\'\'");

  sptr<RomanAtom> ra(new RomanAtom(Formula(tp, str, "mathnormal", false, false)._root));
  ra->_type = AtomType::interText;
  tp.addAtom(ra);
  tp.addRow();

  return nullptr;
}

macro(newcommand) {
  wstring newcmd(args[1]);
  int nbArgs = 0;
  if (!tp.isValidName(newcmd))
    throw ex_parse("Invalid name for the command '" + wide2utf8(newcmd.c_str()));

  if (!args[3].empty()) valueof(args[3], nbArgs);

  if (args[4].empty()) {
    NewCommandMacro::addNewCommand(newcmd.substr(1), args[2], nbArgs);
  } else {
    NewCommandMacro::addNewCommand(newcmd.substr(1), args[2], nbArgs, args[4]);
  }

  return nullptr;
}

macro(renewcommand) {
  wstring newcmd(args[1]);
  int nbArgs = 0;
  if (!tp.isValidName(newcmd))
    throw ex_parse("Invalid name for the command: " + wide2utf8(newcmd.c_str()));

  if (!args[3].empty()) valueof(args[3], nbArgs);

  if (args[4].empty()) {
    NewCommandMacro::addRenewCommand(newcmd.substr(1), args[2], nbArgs);
  } else {
    NewCommandMacro::addRenewCommand(newcmd.substr(1), args[2], nbArgs, args[4]);
  }

  return nullptr;
}

macro(raisebox) {
  auto[ru, r] = SpaceAtom::getLength(args[1]);
  auto[hu, h] = SpaceAtom::getLength(args[3]);
  auto[du, d] = SpaceAtom::getLength(args[4]);

  return sptr<Atom>(new RaiseAtom(Formula(tp, args[2])._root, ru, r, hu, h, du, d));
}

macro(definecolor) {
  color c = TRANSPARENT;
  string cs;
  wide2utf8(args[3].c_str(), cs);
  if (args[2] == L"gray") {
    float f = 0;
    valueof(args[3], f);
    c = rgb(f, f, f);
  } else if (args[2] == L"rgb") {
    StrTokenizer stok(cs, ":,");
    if (stok.count() != 3)
      throw ex_parse("The color definition must have three components!");
    float r, g, b;
    string R = stok.next(), G = stok.next(), B = stok.next();
    valueof(trim(R), r);
    valueof(trim(G), g);
    valueof(trim(B), b);
    c = rgb(r, g, b);
  } else if (args[2] == L"cmyk") {
    StrTokenizer stok(cs, ":,");
    if (stok.count() != 4)
      throw ex_parse("The color definition must have four components!");
    float cmyk[4];
    for (float& i : cmyk) {
      string X = stok.next();
      valueof(trim(X), i);
    }
    float k = 1 - cmyk[3];
    c = rgb(k * (1 - cmyk[0]), k * (1 - cmyk[1]), k * (1 - cmyk[2]));
  } else {
    throw ex_parse("Color model is incorrect!");
  }

  ColorAtom::defineColor(wide2utf8(args[1].c_str()), c);
  return nullptr;
}

macro(sizes) {
  float f = 1;
  if (args[0] == L"tiny")
    f = 0.5f;
  else if (args[0] == L"scriptsize")
    f = 0.7f;
  else if (args[0] == L"footnotesize")
    f = 0.8f;
  else if (args[0] == L"small")
    f = 0.9f;
  else if (args[0] == L"normalsize")
    f = 1.f;
  else if (args[0] == L"large")
    f = 1.2f;
  else if (args[0] == L"Large")
    f = 1.4f;
  else if (args[0] == L"LARGE")
    f = 1.8f;
  else if (args[0] == L"huge")
    f = 2.f;
  else if (args[0] == L"Huge")
    f = 2.5f;

  auto a = Formula(tp, tp.getOverArgument(), "", false, tp.isMathMode())._root;
  return sptr<Atom>(new MonoScaleAtom(a, f));
}

macro(romannumeral) {
  int numbers[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
  string letters[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
  string roman;

  int num;
  string x;
  wide2utf8(args[1].c_str(), x);
  valueof(trim(x), num);
  for (int i = 0; i < 13; i++) {
    while (num >= numbers[i]) {
      roman += letters[i];
      num -= numbers[i];
    }
  }

  if (args[0][0] == 'r') {
    tolower(roman);
  }

  wstring str;
  utf82wide(roman.c_str(), str);
  return Formula(str, false)._root;
}

macro(muskips) {
  SpaceType type = SpaceType::none;
  if (args[0] == L",")
    type = SpaceType::thinMuSkip;
  else if (args[0] == L":")
    type = SpaceType::medMuSkip;
  else if (args[0] == L";")
    type = SpaceType::thickMuSkip;
  else if (args[0] == L"thinspace")
    type = SpaceType::thinMuSkip;
  else if (args[0] == L"medspace")
    type = SpaceType::medMuSkip;
  else if (args[0] == L"thickspace")
    type = SpaceType::thickMuSkip;
  else if (args[0] == L"!")
    type = SpaceType::negThinMuSkip;
  else if (args[0] == L"negthinspace")
    type = SpaceType::negThinMuSkip;
  else if (args[0] == L"negmedspace")
    type = SpaceType::negMedMuSkip;
  else if (args[0] == L"negthickspace")
    type = SpaceType::negThickMuSkip;

  return sptr<Atom>(new SpaceAtom(type));
}

macro(xml) {
  map<string, string>& m = tp._formula->_xmlMap;
  wstring str(args[1]);
  wstring buf;
  size_t start = 0;
  size_t pos;
  while ((pos = str.find(L'$')) != wstring::npos) {
    if (pos < str.length() - 1) {
      start = pos;
      while (++start < str.length() && isalpha(str[start]));
      wstring key = str.substr(pos + 1, start - pos - 1);
      string x = wide2utf8(key.c_str());
      auto it = m.find(x);
      if (it != m.end()) {
        buf.append(str.substr(0, pos));
        wstring x = utf82wide(it->second.c_str());
        buf.append(x);
      } else {
        buf.append(str.substr(0, start));
      }
      str = str.substr(start);
    } else {
      buf.append(str);
      str = L"";
    }
  }
  buf.append(str);
  str = buf;

  return Formula(tp, str)._root;
}

}  // namespace tex
