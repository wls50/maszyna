/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <string>

#include "scenenode.h"
#include "Segment.h"
#include "material.h"
#include "names.h"

class TTractionPowerSource;

class TTraction : public editor::basic_node {

    friend class opengl_renderer;

  public: // na razie
    TTractionPowerSource *psPower[ 2 ] { nullptr, nullptr }; // najbliższe zasilacze z obu kierunków
    TTractionPowerSource *psPowered { nullptr }; // ustawione tylko dla bezpośrednio zasilanego przęsła
    TTraction *hvNext[ 2 ] { nullptr, nullptr }; //łączenie drutów w sieć
    int iNext[ 2 ] { 0, 0 }; // do którego końca się łączy
    int iLast { 1 }; //że niby ostatni drut // ustawiony bit 0, jeśli jest ostatnim drutem w sekcji; bit1 - przedostatni
  public:
    glm::dvec3 pPoint1, pPoint2, pPoint3, pPoint4;
    glm::dvec3 vParametric; // współczynniki równania parametrycznego odcinka
    double fHeightDifference { 0.0 };
    int iNumSections { 0 };
    float NominalVoltage { 0.0f };
    float MaxCurrent { 0.0f };
    float fResistivity { 0.0f }; //[om/m], przeliczone z [om/km]
    DWORD Material { 0 }; // 1: Cu, 2: Al
    float WireThickness { 0.0f };
    DWORD DamageFlag { 0 }; // 1: zasniedziale, 128: zerwana
    int Wires { 2 };
    float WireOffset { 0.0f };
    std::string asPowerSupplyName; // McZapkie: nazwa podstacji trakcyjnej
    TTractionPowerSource *psSection { nullptr }; // zasilacz (opcjonalnie może to być pulpit sterujący EL2 w hali!)
    std::string asParallel; // nazwa przęsła, z którym może być bieżnia wspólna
    TTraction *hvParallel { nullptr }; // jednokierunkowa i zapętlona lista przęseł ewentualnej bieżni wspólnej
    float fResistance[ 2 ] { -1.0f, -1.0f }; // rezystancja zastępcza do punktu zasilania (0: przęsło zasilane, <0: do policzenia)
    int iTries { 0 };
    int PowerState { 0 }; // type of incoming power, if any
    // visualization data
    glm::dvec3 m_origin;
    geometry_handle m_geometry;

    TTraction( scene::node_data const &Nodedata );
    // legacy constructor
    TTraction( std::string Name );

    void Load( cParser *parser, glm::dvec3 const &pOrigin );
    // set origin point
    void
        origin( glm::dvec3 Origin ) {
            m_origin = Origin; }
    // retrieves list of the track's end points
    std::vector<glm::dvec3>
        endpoints() const;

    // creates geometry data in specified geometry bank. returns: number of created elements, or NULL
    // NOTE: deleting nodes doesn't currently release geometry data owned by the node. TODO: implement erasing individual geometry chunks and banks
#ifdef EU07_USE_OLD_GROUNDCODE
    std::size_t create_geometry( geometrybank_handle const &Bank, glm::dvec3 const &Origin ); // wypełnianie VBO
#else
    std::size_t create_geometry( geometrybank_handle const &Bank );
#endif
    int TestPoint(glm::dvec3 const &Point);
    void Connect(int my, TTraction *with, int to);
    void Init();
    bool WhereIs();
    void ResistanceCalc(int d = -1, double r = 0, TTractionPowerSource *ps = nullptr);
    void PowerSet(TTractionPowerSource *ps);
    double VoltageGet(double u, double i);
private:
    glm::vec3 wire_color() const;
};



// collection of virtual tracks and roads present in the scene
class traction_table : public basic_table<TTraction> {

public:
    // legacy method, initializes traction after deserialization from scenario file
    void
        InitTraction();
};

//---------------------------------------------------------------------------
