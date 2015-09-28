// The MIT License (MIT)
//
// Copyright (c) 2015 LNLS Accelerator Division
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <trackcpp/auxiliary.h>
#include <trackcpp/elements.h>
#include <cfloat>

const std::vector<double> Element::default_polynom = std::vector<double>(3,0);


// default constructor (constructs a drift element)
Element::Element(const std::string& fam_name_, const double& length_) :
    fam_name(fam_name_), length(length_) {
  for(unsigned int i=0; i<6; i++) {
    t_in[i] = t_out[i] = 0.0;
    for(unsigned int j=0; j<6; ++j) {
      if (i == j) {
        r_in[i*6+j] = r_out[i*6+j] = 1.0;
      } else {
        r_in[i*6+j] = r_out[i*6+j] = 0.0;
      }
    }
  }

}

const std::string& Element::get_pass_method() {
    return pm_dict[pass_method];
}

void Element::set_pass_method(const std::string &pass_method_) {
    int i;
    for(i = 0; i<pm_dict.size(); i++)
        if (pm_dict[i] == pass_method_)
            break;
    if (i < pm_dict.size())
        pass_method = i;
}


Element Element::marker (const std::string& fam_name_) {
  Element e = Element(fam_name_, 0);
    initialize_marker(e);
  return e;
}

Element Element::bpm (const std::string& fam_name_) {
    Element e = Element(fam_name_, 0);
    initialize_marker(e);
    return e;
}

Element Element::drift (const std::string& fam_name_, const double& length_) {
  Element e = Element(fam_name_, length_);
    initialize_drift(e);
  return e;
}

Element Element::hcorrector(const std::string& fam_name_, const double& length_, const double& hkick_) {
  Element e = Element(fam_name_, length_);
    initialize_corrector(e, hkick_, 0.0);
  return e;
}

Element Element::vcorrector(const std::string& fam_name_, const double& length_, const double& vkick_) {
  Element e = Element(fam_name_, length_);
    initialize_corrector(e, 0.0, vkick_);
  return e;
}

Element Element::corrector(const std::string& fam_name_, const double& length_, const double& hkick_, const double& vkick_) {
  Element e = Element(fam_name_, length_);
    initialize_corrector(e, hkick_, vkick_);
  return e;
}

Element Element::quadrupole (const std::string& fam_name_, const double& length_, const double& K_, const int nr_steps_) {
  Element e = Element(fam_name_, length_);
    initialize_quadrupole(e, K_, nr_steps_);
  return e;
}

Element Element::sextupole (const std::string& fam_name_, const double& length_, const double& S_, const int nr_steps_) {
  Element e = Element(fam_name_, length_);
    initialize_sextupole(e, S_, nr_steps_);
  return e;
}

Element Element::rbend (const std::string& fam_name_, const double& length_,
    const double& angle_, const double& angle_in_, const double& angle_out_,
    const double& gap_, const double& fint_in_, const double& fint_out_,
    const std::vector<double>& polynom_a_, const std::vector<double>& polynom_b_,
    const double& K_, const double& S_, const int nr_steps_) {
    Element e = Element(fam_name_, length_);
    initialize_rbend(e, angle_, angle_in_, angle_out_, gap_, fint_in_, fint_out_, polynom_a_, polynom_b_, K_, S_, nr_steps_);
  return e;
}

Element Element::rfcavity (const std::string& fam_name_, const double& length_, const double& frequency_, const double& voltage_) {
  Element e = Element(fam_name_, length_);
    initialize_rfcavity(e, frequency_, voltage_);
  return e;
}



void print_polynom(std::ostream& out, const std::string& label, const std::vector<double>& polynom) {
  int order = 0;
  for(unsigned int i=0; i<polynom.size(); ++i) {
    if (polynom[i] != 0) order = i+1;
  }
  if (order > 0) out << std::endl << label;
  for(int i=0; i<order; ++i) {
    out << polynom[i] << " ";
  }
}

bool Element::operator==(const Element& o) const {

    if (this == &o) return true;
    if (this->fam_name != o.fam_name) return false;
    if (this->pass_method != o.pass_method) return false;
    if (this->length != o.length) return false;
    if (this->hmin != o.hmin) return false;
    if (this->hmax != o.hmax) return false;
    if (this->vmin != o.vmin) return false;
    if (this->vmax != o.vmax) return false;
    if (this->nr_steps != o.nr_steps) return false;

    // the optimization bellow breaks true object comparison but
    // keeps physical comparison between elements.
    if ((this->pass_method == PassMethod::pm_drift_pass) or
       (this->pass_method == PassMethod::pm_identity_pass)) return true;

    if (this->hkick != o.hkick) return false;
    if (this->vkick != o.vkick) return false;
    if (this->angle != o.angle) return false;
    if (this->angle_in != o.angle_in) return false;
    if (this->angle_out != o.angle_out) return false;
    if (this->gap != o.gap) return false;
    if (this->fint_in != o.fint_in) return false;
    if (this->fint_out != o.fint_out) return false;
    if (this->thin_KL != o.thin_KL) return false;
    if (this->thin_SL != o.thin_SL) return false;
    if (this->frequency != o.frequency) return false;
    if (this->voltage != o.voltage) return false;
    if (this->polynom_a != o.polynom_a) return false;
    if (this->polynom_b != o.polynom_b) return false;
    for(unsigned int i=0; i<6; ++i) {
      if (this->t_in[i] != o.t_in[i]) return false;
      if (this->t_out[i] != o.t_out[i]) return false;
    }
    for(unsigned int i=0; i<36; ++i) {
      if (this->r_in[i] != o.r_in[i]) return false;
      if (this->r_out[i] != o.r_out[i]) return false;
    }
    if ((this->kicktable != nullptr) and (o.kicktable == nullptr)) return false;
    if ((this->kicktable == nullptr) and (o.kicktable != nullptr)) return false;
    if ((this->kicktable != nullptr) and (o.kicktable != nullptr) and (*(this->kicktable) != *(o.kicktable))) return false;

    return true;

}

std::ostream& operator<< (std::ostream &out, const Element& el) {

                        out              << "fam_name      : " << el.fam_name;
  if (el.length != 0)   out << std::endl << "length        : " << el.length;
                        out << std::endl << "pass_method   : " << pm_dict[el.pass_method];
  if (el.nr_steps > 1)  out << std::endl << "nr_steps      : " << el.nr_steps;
  if (el.thin_KL != 0)  out << std::endl << "thin_KL       : " << el.thin_KL;
  if (el.thin_SL != 0)  out << std::endl << "thin_SL       : " << el.thin_SL;
  if (el.angle != 0)    out << std::endl << "bending_angle : " << el.angle;
  if (el.angle != 0)    out << std::endl << "entrance_angle: " << el.angle_in;
  if (el.angle != 0)    out << std::endl << "exit_angle    : " << el.angle_out;
  if ((el.gap != 0) and ((el.fint_in != 0) or (el.fint_out != 0))) {
                        out << std::endl << "gap           : " << el.gap;
                        out << std::endl << "fint_in       : " << el.fint_in;
                        out << std::endl << "fint_out      : " << el.fint_out;
  }
  print_polynom(        out,                "polynom_a     : ", el.polynom_a);
  print_polynom(        out,                "polynom_b     : ", el.polynom_b);
  if (el.frequency != 0)out << std::endl << "frequency     : " << el.frequency;
  if (el.voltage != 0)  out << std::endl << "voltage       : " << el.voltage;
  return out;
}

void initialize_marker(Element &element) {
    element.pass_method = PassMethod::pm_identity_pass;
}

void initialize_corrector(Element &element, const double &hkick, const double &vkick) {
    element.pass_method = PassMethod::pm_corrector_pass;
    element.hkick = hkick;
    element.vkick = vkick;
}

void initialize_drift(Element &element) {
    element.pass_method = PassMethod::pm_drift_pass;
}

void initialize_rbend(Element& element, const double& angle, const double& angle_in, const double& angle_out, const double& gap, const double& fint_in, const double& fint_out, const std::vector<double>& polynom_a, const std::vector<double>& polynom_b, const double& K, const double& S, const int nr_steps) {
    element.pass_method = PassMethod::pm_bnd_mpole_symplectic4_pass;
    element.angle = angle;
    element.angle_in = angle_in;
    element.angle_out = angle_out;
    element.gap = gap;
    element.fint_in = fint_in;
    element.fint_out = fint_out;
    element.polynom_a = polynom_a;
    element.polynom_b = polynom_b;
    element.polynom_b[1] = K;
    element.polynom_b[2] = S;
    element.nr_steps = nr_steps;
}



void initialize_quadrupole(Element &element, const double &K, const int &nr_steps) {
    element.pass_method = PassMethod::pm_str_mpole_symplectic4_pass;
    element.polynom_b[1] = K;
    element.nr_steps = nr_steps;
}

void initialize_sextupole(Element &element, const double &S, const int &nr_steps) {
    element.pass_method = PassMethod::pm_str_mpole_symplectic4_pass;
    element.polynom_b[2] = S;
    element.nr_steps = nr_steps;
}

void initialize_rfcavity(Element &element, const double &frequency, const double &voltage) {
    element.pass_method = PassMethod::pm_cavity_pass;
    element.frequency = frequency;
    element.voltage = voltage;
}