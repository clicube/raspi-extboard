import Vue from 'vue'
import Vuex from 'vuex'
import envs from './envs'

Vue.use(Vuex)
export default new Vuex.Store({
    modules: {
        envs: envs
    }
})
