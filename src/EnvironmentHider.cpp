#include "main.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "GlobalNamespace/LightManager.hpp"
#include "GlobalNamespace/MultiplayerLobbyAvatarPlace.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp" 
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "custom-types/shared/coroutine.hpp"

#include "System/Collections/IEnumerator.hpp"
#include "UnityEngine/WaitForSeconds.hpp"

#include "GlobalNamespace/MenuEnvironmentManager.hpp"
#include "GlobalNamespace/MenuEnvironmentManager_MenuEnvironmentObjects.hpp"

#include <regex>
#include <list>

using namespace UnityEngine;

bool getBGActive()
{
    return (backgroundObject && backgroundObject->get_activeSelf());
}

UnityEngine::GameObject* FindMultiplayerPlatform()
{
    ArrayW<GlobalNamespace::MultiplayerLobbyAvatarPlace*> platforms = Resources::FindObjectsOfTypeAll<GlobalNamespace::MultiplayerLobbyAvatarPlace*>();
    for (size_t i = 0; i < platforms->Length(); i++)
    {
        GlobalNamespace::MultiplayerLobbyAvatarPlace* platform = platforms[i];
        UnityEngine::Transform* plat = platform->get_transform();
        if (plat->get_parent() != nullptr) {
            return plat->get_parent()->get_gameObject();
        }
    }
    return nullptr;
}

void HideChildRenderers(GameObject* obj, bool onlymeshes, bool unhide = false, bool ignorelayer = false)
{
    if (obj == nullptr) return;
    bool enabled = !getBGActive() || unhide;
    ArrayW<UnityEngine::Renderer*> rendarray = onlymeshes ? (ArrayW<Renderer*>)obj->GetComponentsInChildren<MeshRenderer*>() : obj->GetComponentsInChildren<Renderer*>();
    for (size_t i = 0; i < rendarray->Length(); i++)
    {
        Renderer* renderer = (Renderer*)rendarray->values[i];
        if (!ignorelayer && renderer->get_gameObject()->get_layer() == 13) continue;
        if (!renderer || !renderer->m_CachedPtr) continue;
        renderer->set_enabled(enabled);
    }
}

custom_types::Helpers::Coroutine HideChildLights(GameObject* obj, bool unhide = false)
{
    if (obj == nullptr) co_return;
    co_yield reinterpret_cast<System::Collections::IEnumerator*>(CRASH_UNLESS(UnityEngine::WaitForSeconds::New_ctor(0.5f)));
    ArrayW<Renderer*> rendarray = obj->GetComponentsInChildren<Renderer*>();
    for (size_t i = 0; i < rendarray->Length(); i++)
    {
        Renderer* renderer = (Renderer*)rendarray->values[i];
        if (!renderer || !renderer->m_CachedPtr || renderer->GetComponent<GlobalNamespace::LightManager*>()) continue;
        if (!std::regex_search(to_utf8(csstrtostr(renderer->get_name())), std::regex("bloom|light", std::regex::icase)) && renderer->get_gameObject()->get_layer() != 13) continue;
        renderer->set_enabled(unhide);
    }
    co_return;
}

void HideMenuEnv()
{
    bool ismulti = Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuEnvironmentManager*>()->values[0]->prevMenuEnvironmentType == (int)2;
    bool bgActive = getBGActive();
    if (!ismulti)
    {
        // Find Objects
        auto* floorObj = GameObject::Find(il2cpp_utils::newcsstr("MenuEnvironmentManager/DefaultMenuEnvironment/BasicMenuGround"));
        auto* notesObj = GameObject::Find(il2cpp_utils::newcsstr("MenuEnvironmentManager/DefaultMenuEnvironment/Notes"));
        auto* notePileObj = GameObject::Find(il2cpp_utils::newcsstr("MenuEnvironmentManager/DefaultMenuEnvironment/PileOfNotes"));
        auto* multiEnvObj = FindMultiplayerPlatform();
        
        // Apply Visibility
        multiEnvObj->SetActive(bgActive);
        HideChildRenderers(notesObj, false);
        HideChildRenderers(notePileObj, false);
        if (floorObj) floorObj->GetComponent<MeshRenderer*>()->set_enabled(!bgActive);
        if (bgActive) {
            HideChildRenderers(multiEnvObj, false, false, true);
            HideChildRenderers(multiEnvObj->Find(il2cpp_utils::newcsstr("LobbyAvatarPlace")), false, true);
        }
    }
}

void HideGameEnv()
{
    GameObject* environmentObj = GameObject::Find(il2cpp_utils::newcsstr("Environment"));
    GameObject* playersPlaceObj = GameObject::Find(il2cpp_utils::newcsstr("Environment/PlayersPlace"));
    GameObject* coreLightingObj = GameObject::Find(il2cpp_utils::newcsstr("Environment/CoreLighting"));
    
    if (getConfig().config["hideLasers"].GetBool())
    {
        auto* behaviour = environmentObj->GetComponent<MonoBehaviour*>();
        behaviour->StartCoroutine("HideChildLights", environmentObj);
    }
    if (getConfig().config["hideEnvironment"].GetBool())
    {
        HideChildRenderers(environmentObj, false);
        HideChildRenderers(playersPlaceObj, true, true);
    }
    HideChildRenderers(coreLightingObj, true, true);
}