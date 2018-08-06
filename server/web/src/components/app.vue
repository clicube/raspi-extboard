<template>
    <div id="app">
        <button type="button" @click="update()">update!!</button>
        <status/>
    </div>
</template>

<script>

    function update(store){
        store.dispatch('envs/update')
    }

    import status from './status'
    export default {
        components: { status },
        methods: {
            update: function() {
                update(this.$store)
            }
        },
        mounted: function() {
            update(this.$store)
        }
    }
</script>

<style scoped>
</style>

