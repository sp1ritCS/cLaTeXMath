#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include <cstring>

#include "common.h"
#include "fonts/fonts.h"

namespace tex {

class Box;

#ifdef HAVE_LOG

void print_box(const sptr<Box>& box);

#endif  // HAVE_LOG

class BoxSplitter {
public:
  struct Position {
    int _index;
    sptr<HorizontalBox> _box;

    Position(int index, const sptr<HorizontalBox>& box)
      : _index(index), _box(box) {}
  };

private:
  static float canBreak(std::stack<Position>& stack, const sptr<HorizontalBox>& hbox, float width);

  static int getBreakPosition(const sptr<HorizontalBox>& hb, int index);

public:
  static sptr<Box> split(const sptr<Box>& box, float width, float lineSpace);

  static sptr<Box> split(const sptr<HorizontalBox>& hb, float width, float lineSpace);
};

/**
 * Contains the used TeXFont-object, color settings and the current style in
 * which a formula must be drawn. It's used in the createBox-methods. Contains
 * methods that apply the style changing rules for subformula's.
 */
class Environment {
private:
  // current style
  TexStyle _style;
  // TeXFont used
  sptr<TeXFont> _tf;
  // last used font
  int _lastFontId;
  // Environment width
  float _textWidth;

  // The text style to use
  std::string _textStyle;
  // If is small capital
  bool _smallCap;
  float _scaleFactor;
  // The unit of inter-line space
  UnitType _interlineUnit;
  // The inter line space
  float _interline;

  // Member to store copies to prevent destruct
  sptr<Environment> _copy, _copytf, _cramp, _dnom;
  sptr<Environment> _num, _root, _sub, _sup;

  inline void init() {
    _style = TexStyle::display;
    _lastFontId = TeXFont::NO_FONT;
    _textWidth = POS_INF;
    _smallCap = false;
    _scaleFactor = 1.f;
    _interlineUnit = UnitType::em;
    _interline = 0;
  }

  Environment(TexStyle style, const sptr<TeXFont>& tf, color bg, const color c) {
    init();
    _style = style;
    _tf = tf;
    setInterline(UnitType::ex, 1.f);
  }

  Environment(
    TexStyle style, float scaleFactor,
    const sptr<TeXFont>& tf,
    const std::string& textstyle, bool smallCap  //
  ) {
    init();
    _style = style;
    _scaleFactor = scaleFactor;
    _tf = tf;
    _textStyle = textstyle;
    _smallCap = smallCap;
    setInterline(UnitType::ex, 1.f);
  }

public:
  Environment(TexStyle style, const sptr<TeXFont>& tf) {
    init();
    _style = style;
    _tf = tf;
    setInterline(UnitType::ex, 1.f);
  }

  Environment(TexStyle style, const sptr<TeXFont>& tf, UnitType widthUnit, float textWidth);

  inline void setInterline(UnitType unit, float len) {
    _interline = len;
    _interlineUnit = unit;
  }

  float getInterline() const;

  void setTextWidth(UnitType widthUnit, float width);

  inline float getTextWidth() const { return _textWidth; }

  inline void setScaleFactor(float f) { _scaleFactor = f; }

  inline float getScaleFactor() const { return _scaleFactor; }

  sptr<Environment>& copy();

  sptr<Environment>& copy(const sptr<TeXFont>& tf);

  /**
   * Copy of this envrionment in cramped style.
   */
  sptr<Environment>& crampStyle();

  /**
   * Style to display denominator.
   */
  sptr<Environment>& dnomStyle();

  /**
   * Style to display numerator.
   */
  sptr<Environment>& numStyle();

  /**
   * Style to display roots.
   */
  sptr<Environment>& rootStyle();

  /**
   * Style to display subscripts.
   */
  sptr<Environment>& subStyle();

  /**
   * Style to display superscripts.
   */
  sptr<Environment>& supStyle();

  inline float getSize() const { return _tf->getSize(); }

  inline TexStyle getStyle() const { return _style; }

  inline void setStyle(TexStyle style) { _style = style; }

  inline const std::string& getTextStyle() const { return _textStyle; }

  inline void setTextStyle(const std::string& style) { _textStyle = style; }

  inline bool getSmallCap() const { return _smallCap; }

  inline void setSmallCap(bool s) { _smallCap = s; }

  inline const sptr<TeXFont>& getTeXFont() const { return _tf; }

  inline float getSpace() const { return _tf->getSpace(_style) * _tf->getScaleFactor(); }

  inline void setLastFontId(int id) { _lastFontId = id; }

  inline int getLastFontId() const {
    return (_lastFontId == TeXFont::NO_FONT ? _tf->getMuFontId() : _lastFontId);
  }
};

/**
 * Represents glue by its 3 components. Contains the "glue rules"
 */
class Glue {
private:
  // contains the different glue types
  static std::vector<Glue*> _glueTypes;
#define TYPE_COUNT  8
#define STYLE_COUNT 5
  // the glue table represents the "glue rules"
  static const char _table[TYPE_COUNT][TYPE_COUNT][STYLE_COUNT];
  // the glue components
  float _space;
  float _stretch;
  float _shrink;
  std::string _name;

  sptr<Box> createBox(const Environment& env) const;

  static float getFactor(const Environment& env) ;

  static Glue* getGlue(SpaceType skipType);

  static int getGlueIndex(AtomType ltype, AtomType rtype, const Environment& env);

public:
  Glue() = delete;

  Glue(float space, float stretch, float shrink, const std::string& name) {
    _space = space;
    _stretch = stretch;
    _shrink = shrink;
    _name = name;
  }

  inline const std::string& getName() const {
    return _name;
  }

  /**
   * Creates a box representing the glue type according to the "glue rules" based
   * on the atom types between which the glue must be inserted.
   *
   * @param ltype
   *      left atom type
   * @param rtype
   *      right atom type
   * @param env
   *      the Environment
   * @return a box containing representing the glue
   */
  static sptr<Box> get(AtomType ltype, AtomType rtype, const Environment& env);

  /**
   * Creates a box representing the glue type according to the "glue rules" based
   * on the skip-type
   */
  static sptr<Box> get(SpaceType skipType, const Environment& env);

  /**
   * Get the space amount from the given left-type and right-type of atoms
   * according to the "glue rules".
   */
  static float getSpace(AtomType ltype, AtomType rtype, const Environment& env);

  /**
   * Get the space amount from the given skip-type according to the "glue rules"
   */
  static float getSpace(SpaceType skipType, const Environment& env);

  static void _init_();

  static void _free_();

#ifdef HAVE_LOG

  friend std::ostream& operator<<(std::ostream& out, const Glue& glue);

#endif  // HAVE_LOG
};

}  // namespace tex

#endif  // CORE_H_INCLUDED
