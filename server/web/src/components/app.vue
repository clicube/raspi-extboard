<template>
    <div id="app">
        <status/>
        <button type="button" @click="update()">update!!</button>
    </div>
</template>

<script>
    import status from './status'
    export default {
        components: { status },
        methods: {
            update: function() {
                console.log("update click")
                this.$store.dispatch('envs/update')
            }
        }
    }
</script>

<style scoped>
</style>

